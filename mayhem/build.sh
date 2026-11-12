#!/usr/bin/env bash
#
# mayhem/build.sh — builds the Contiki-NG fuzz harnesses (sanitized, DWARF 3), their standalone
# reproducers, and the upstream native unit tests (normal flags) that mayhem/test.sh runs.
#
# Runs inside the commit image (mayhem/Dockerfile) as `mayhem` in /mayhem; offline and re-runnable.
set -euo pipefail

# clang rejects SOURCE_DATE_EPOCH='' (empty) — it must be unset or a valid integer.
[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH

: "${SANITIZER_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer}"
: "${DEBUG_FLAGS:=-g -gdwarf-3}"
: "${CC:=clang}" ; : "${LIB_FUZZING_ENGINE:=-fsanitize=fuzzer}"
: "${MAYHEM_JOBS:=$(nproc)}"
: "${COVERAGE_FLAGS=}"
export SANITIZER_FLAGS DEBUG_FLAGS CC LIB_FUZZING_ENGINE MAYHEM_JOBS COVERAGE_FLAGS

cd "$SRC"

HARNESSES="fuzz_uip fuzz_sicslowpan fuzz_coap fuzz_lwm2m_tlv fuzz_jsonparse fuzz_frame802154"
TESTS="28-coap-parse 29-jsonparse 32-lwm2m-tlv 16-cbor 23-uiplib 14-sha-256 11-aes-ccm 19-bitrev 15-ieee802154-security 20-random 17-process-mutex 12-heapmem"
FUZZDIR="$SRC/mayhem/contiki-fuzz"

# Make does not see a flag change as a reason to rebuild: start every run from clean build dirs.
rm -rf "$FUZZDIR/build"
for t in $TESTS; do rm -rf "$SRC/tests/08-native-runs/$t/build"; done

# 1) The unit tests, with the project's NORMAL flags (no sanitizers, no -gdwarf-3, no fuzzer
#    instrumentation), so test.sh only has to run them. Flags go in via the environment: on make's
#    command line they would replace the -I paths Makefile.include appends to CFLAGS.
LOG=$(mktemp)
quiet() { "$@" >"$LOG" 2>&1 || { grep -v 'warning:' "$LOG" | tail -n 40 >&2; exit 1; }; }
for t in $TESTS; do
  quiet env CFLAGS="$COVERAGE_FLAGS" LDFLAGS="$COVERAGE_FLAGS" \
    make -C "$SRC/tests/08-native-runs/$t" -j"$MAYHEM_JOBS" TARGET=native CC="$CC" LD="$CC"
done

# 2) The whole Contiki-NG stack (all modules the harnesses reach) with $SANITIZER_FLAGS, as one
#    multi-project build: every harness shares the same instrumented objects.
#    - Contiki's Makefile.include appends its -I paths to CFLAGS, so flags go in via the environment.
#    - -Dmain=contiki_native_main frees `main` for libFuzzer; the harness replaces os/contiki-main.c's
#      startup (mayhem/contiki-fuzz/fuzz-common.c).
#    - -fno-sanitize=function: Contiki calls callbacks through casted function pointers throughout.
#    - lsan_off.c (LeakSanitizer off via __lsan_is_turned_off) is a PROJECT_SOURCEFILES entry of
#      mayhem/contiki-fuzz/Makefile, so every fuzzer and standalone binary links it.
#    - LDFLAGS_WERROR= : the native link adds --warn-common and --fatal-warnings, and ASan's own
#      common symbols trip it.
FUZZ_CFLAGS="$SANITIZER_FLAGS -fno-sanitize=function $DEBUG_FLAGS -fsanitize=fuzzer-no-link -Dmain=contiki_native_main"
cd "$FUZZDIR"
CFLAGS="$FUZZ_CFLAGS" LDFLAGS="$SANITIZER_FLAGS $DEBUG_FLAGS $LIB_FUZZING_ENGINE" LDFLAGS_WERROR= \
  quiet make -j"$MAYHEM_JOBS" TARGET=native CC="$CC" LD="$CC"
cd "$SRC"
rm -f "$LOG"

# 3) One fuzzer + one standalone (run-once, no libFuzzer runtime) reproducer per harness.
OBJ="$FUZZDIR/build/native/obj"
for h in $HARNESSES; do
  cp "$FUZZDIR/build/native/$h.native" "$SRC/$h"
  shared=$(ls "$OBJ"/*.o | grep -v '/fuzz_[a-z0-9_]*\.o$' | sort)
  # shellcheck disable=SC2086
  $CC $SANITIZER_FLAGS $DEBUG_FLAGS -Wl,-zdefs $shared "$OBJ/$h.o" "$STANDALONE_FUZZ_MAIN" -lrt -o "$SRC/$h-standalone"
done

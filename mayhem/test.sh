#!/usr/bin/env bash
#
# mayhem/test.sh — runs Contiki-NG's own native unit tests (tests/08-native-runs/*, built by
# mayhem/build.sh with normal flags) and checks each one's known-answer assertions.
#
# IMPORTANT:
#  * Must assert BEHAVIOR/OUTPUT, not just exit status. The oracle has to check asserted values /
#    golden-output diffs / known-answer results — so a PATCH that "fixes" a bug by making the program
#    exit(0) (or any no-op) FAILS here. Running inputs and checking only "exit 0 / didn't crash" is
#    NOT a functional test (it's trivially reward-hackable) — use the project's real assertion suite.
#  * Do NOT build here — mayhem/build.sh already compiled the test suite (with the project's normal
#    flags). This script only RUNS the pre-built tests and reports counts. If the test runner is
#    missing, that's a build.sh bug — fail loudly rather than silently rebuilding.
#  * REQUIRED OUTPUT — a CTRF (https://ctrf.io) summary so Mayhem/the PATCH grader reads the counts:
#      - writes a CTRF JSON report to ${CTRF_REPORT:-$SRC/ctrf-report.json}, and
#      - prints a one-line `CTRF {...}` marker to stdout (same JSON, compact).
#    Only `results.summary` (with tests/passed/failed/pending/skipped/other) is required.
#    Use the emit_ctrf helper below; it computes tests = passed+failed+skipped and sets the exit
#    code (0 iff failed==0). Map your framework's output to passed/failed/skipped.
set -uo pipefail
[ -n "${SOURCE_DATE_EPOCH:-}" ] || unset SOURCE_DATE_EPOCH
: "${MAYHEM_JOBS:=$(nproc)}"   # build parallelism; env-overridable, falls back to nproc (use -j"$MAYHEM_JOBS")
cd "$SRC"

# emit_ctrf <tool> <passed> <failed> [skipped] [pending] [other]
# Writes a CTRF report (file + stdout `CTRF {...}` marker) and returns non-zero iff failed>0.
emit_ctrf() {
  local tool="$1" passed="$2" failed="$3" skipped="${4:-0}" pending="${5:-0}" other="${6:-0}"
  local tests=$(( passed + failed + skipped + pending + other ))
  cat > "${CTRF_REPORT:-$SRC/ctrf-report.json}" <<JSON
{
  "results": {
    "tool": { "name": "$tool" },
    "summary": {
      "tests": $tests,
      "passed": $passed,
      "failed": $failed,
      "pending": $pending,
      "skipped": $skipped,
      "other": $other
    }
  }
}
JSON
  printf 'CTRF {"results":{"tool":{"name":"%s"},"summary":{"tests":%d,"passed":%d,"failed":%d,"pending":%d,"skipped":%d,"other":%d}}}\n' \
    "$tool" "$tests" "$passed" "$failed" "$pending" "$skipped" "$other"
  [ "$failed" -eq 0 ]
}


# Each test is a Contiki process that runs its unit-test suites, prints one "Result: success|failure"
# line per suite, then "=check-me= DONE" and keeps idling (so it never exits on its own).
# <dir>:<binary>:<number of suites that must report success>
TESTS="
28-coap-parse:test-coap-parse:8
29-jsonparse:test-jsonparse:6
32-lwm2m-tlv:test-lwm2m-tlv:5
16-cbor:test-cbor:4
23-uiplib:test-uiplib:23
14-sha-256:test-sha-256:10
11-aes-ccm:test-aesccm:2
19-bitrev:test-bitrev:4
15-ieee802154-security:test-ieee802154-security:3
20-random:test-random:4
17-process-mutex:test-process-mutex:2
12-heapmem:test-heapmem:9
"

passed=0; failed=0
for entry in $TESTS; do
  IFS=: read -r dir bin want <<<"$entry"
  exe="$SRC/tests/08-native-runs/$dir/build/native/$bin.native"
  log=$(mktemp)
  if [ ! -x "$exe" ]; then
    echo "FAIL $bin: $exe was not built" >&2
    failed=$((failed + 1)); continue
  fi
  (cd "$(dirname "$exe")/../.." && exec timeout 60 "$exe" </dev/null >"$log" 2>&1) &
  pid=$!
  for _ in $(seq 1 300); do
    grep -q '=check-me= DONE' "$log" && break
    kill -0 "$pid" 2>/dev/null || break
    sleep 0.1
  done
  pkill -P "$pid" 2>/dev/null; kill "$pid" 2>/dev/null; wait "$pid" 2>/dev/null
  ok=$(grep -c '^Result: success' "$log")
  bad=$(grep -c '^Result: failure' "$log")
  if grep -q '=check-me= DONE' "$log" && [ "$bad" -eq 0 ] && [ "$ok" -eq "$want" ]; then
    echo "PASS $bin: $ok/$want suites succeeded"
    passed=$((passed + 1))
  else
    echo "FAIL $bin: $ok succeeded (want $want), $bad failed, done-marker: $(grep -c '=check-me= DONE' "$log")" >&2
    tail -n 5 "$log" >&2
    failed=$((failed + 1))
  fi
  rm -f "$log"
done

emit_ctrf "contiki-ng-native-unit-tests" "$passed" "$failed" 0

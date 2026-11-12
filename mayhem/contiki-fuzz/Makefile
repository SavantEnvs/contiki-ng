# Contiki-NG native build of the fuzz harnesses. Driven by mayhem/build.sh, which supplies
# CC/LD/CFLAGS/LDFLAGS. All harnesses share one set of sanitized Contiki objects.
CONTIKI_PROJECT = fuzz_uip fuzz_sicslowpan fuzz_coap fuzz_lwm2m_tlv fuzz_jsonparse fuzz_frame802154
all: $(CONTIKI_PROJECT)

TARGET = native

CONTIKI = ../..

MODULES += os/net/app-layer/coap
MODULES += os/lib/json

PROJECTDIRS += $(CONTIKI)/os/services/lwm2m
PROJECT_SOURCEFILES += lwm2m-tlv.c fuzz-common.c lsan_off.c
# Per-input reset needs three things upstream keeps private: a clock that stands still (the
# harness provides it), sicslowpan's reassembly contexts and CoAP's transaction list (the
# harness compiles the upstream .c plus a reset function, in place of the original).
PROJECT_SOURCEFILES += fuzz-sicslowpan-state.c fuzz-coap-state.c
CONTIKI_SOURCES_EXCLUDES += clock.c sicslowpan.c coap-transactions.c

include $(CONTIKI)/Makefile.include

# Benign UB in upstream, relaxed on the one object each (everything else stays halting):
# memmove(dst, NULL, 0) when serializing a payload-less message, and a signed
# 0xff << 24 when sign-extending a TLV integer.
$(OBJECTDIR)/coap.o: CFLAGS += -fno-sanitize=nonnull-attribute
$(OBJECTDIR)/lwm2m-tlv.o: CFLAGS += -fno-sanitize=shift-base

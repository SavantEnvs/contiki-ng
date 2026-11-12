#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "contiki.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uiplib.h"
#include "coap.h"
#include "coap-engine.h"

#include "fuzz-common.h"

static coap_endpoint_t endpoint;
/* The UDP connection coap_engine_init() opened; uip_init() clears the table, so put it back. */
static struct uip_udp_conn udp_conns[UIP_UDP_CONNS];

int
LLVMFuzzerInitialize(int *argc, char ***argv)
{
  fuzz_contiki_init();
  coap_engine_init();
  fuzz_contiki_drain();
  uiplib_ipaddrconv("fdfd::100", &endpoint.ipaddr);
  endpoint.port = 8293;
  endpoint.secure = 0;
  memcpy(udp_conns, uip_udp_conns, sizeof(udp_conns));
  return 0;
}

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  /* A CoAP-over-UDP message arrives in one uIP datagram. */
  static uint8_t message[UIP_BUFSIZE];
  if(size == 0 || size > sizeof(message)) {
    return -1;
  }
  fuzz_contiki_reset();
  memcpy(uip_udp_conns, udp_conns, sizeof(udp_conns));
  fuzz_coap_clear_transactions();
  coap_remove_observer_by_client(&endpoint);
  memcpy(message, data, size);
  coap_receive(&endpoint, message, size);
  fuzz_contiki_drain();
  return 0;
}

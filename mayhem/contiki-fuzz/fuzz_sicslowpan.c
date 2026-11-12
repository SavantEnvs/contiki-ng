#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "contiki.h"
#include "net/ipv6/sicslowpan.h"
#include "net/linkaddr.h"
#include "net/netstack.h"
#include "net/packetbuf.h"

#include "fuzz-common.h"

int
LLVMFuzzerInitialize(int *argc, char ***argv)
{
  fuzz_contiki_init();
  sicslowpan_driver.init();
  return 0;
}

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  /* The radio hands the network layer one frame of at most PACKETBUF_SIZE bytes. */
  if(size == 0 || size > PACKETBUF_SIZE) {
    return -1;
  }
  fuzz_contiki_reset();
  fuzz_sicslowpan_reset();
  static const linkaddr_t sender = {{ 0x00, 0x12, 0x4b, 0x00, 0x06, 0x0d, 0x89, 0x01 }};
  static const linkaddr_t receiver = {{ 0x00, 0x12, 0x4b, 0x00, 0x06, 0x0d, 0x89, 0x02 }};
  packetbuf_clear();
  packetbuf_copyfrom(data, size);
  packetbuf_set_addr(PACKETBUF_ADDR_SENDER, &sender);
  packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &receiver);
  sicslowpan_driver.input();
  fuzz_contiki_drain();
  return 0;
}

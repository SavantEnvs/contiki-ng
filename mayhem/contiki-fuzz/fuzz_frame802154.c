#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "contiki.h"
#include "net/mac/framer/frame802154.h"
#include "net/netstack.h"
#include "net/packetbuf.h"

#include "fuzz-common.h"

int
LLVMFuzzerInitialize(int *argc, char ***argv)
{
  fuzz_contiki_init();
  return 0;
}

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  /* The radio hands the framer one frame of at most PACKETBUF_SIZE bytes. */
  if(size == 0 || size > PACKETBUF_SIZE) {
    return -1;
  }
  packetbuf_clear();
  packetbuf_copyfrom(data, size);
  if(NETSTACK_FRAMER.parse() < 0) {
    return -1;
  }
  return 0;
}

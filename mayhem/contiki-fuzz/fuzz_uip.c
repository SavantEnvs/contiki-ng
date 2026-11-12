#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "contiki.h"
#include "net/ipv6/uip.h"

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
  /* uip_buf holds exactly one packet of at most UIP_BUFSIZE bytes. */
  if(size == 0 || size > sizeof(uip_buf)) {
    return -1;
  }
  fuzz_contiki_reset();
  memcpy(uip_buf, data, size);
  uip_len = size;
  uip_input();
  fuzz_contiki_drain();
  return 0;
}

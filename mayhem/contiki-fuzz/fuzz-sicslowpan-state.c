/* Compiled in place of os/net/ipv6/sicslowpan.c (excluded in the Makefile): the unmodified
   upstream source, plus a reset for its file-static reassembly state, which sicslowpan_init()
   does not clear. */
#include "net/ipv6/sicslowpan.c"

#include <string.h>

#include "fuzz-common.h"

void
fuzz_sicslowpan_reset(void)
{
  memset(frag_info, 0, sizeof(frag_info));
  memset(frag_buf, 0, sizeof(frag_buf));
  my_tag = 0;
  sicslowpan_init();
}

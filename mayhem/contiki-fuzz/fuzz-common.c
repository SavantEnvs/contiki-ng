#include <string.h>
#include "contiki.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/tcpip.h"
#include "net/netstack.h"
#include "net/queuebuf.h"
#include "net/linkaddr.h"
#include "sys/node-id.h"
#include "sys/ctimer.h"
#include "sys/etimer.h"
#include "sys/rtimer.h"
#include "sys/clock.h"
#include "lib/random.h"
#include "sys/autostart.h"
#include "net/nbr-table.h"
#include "net/packetbuf.h"
#include "net/ipv6/uip-ds6.h"
#include "net/routing/routing.h"

#include "fuzz-common.h"

/* contiki-main.c's main() is renamed away (-Dmain=contiki_native_main) but its
   autostart reference still has to resolve. */
struct process * const autostart_processes[] = { NULL };

/* A clock that never advances: no timer can expire, so the outcome of an input does not depend
   on host speed or on how long the process has been running. Replaces
   arch/platform/native/clock.c (excluded in the Makefile). */
#define FUZZ_CLOCK_SECONDS 1000

clock_time_t
clock_time(void)
{
  return FUZZ_CLOCK_SECONDS * CLOCK_SECOND;
}

unsigned long
clock_seconds(void)
{
  return FUZZ_CLOCK_SECONDS;
}

void
clock_delay(unsigned int d)
{
}

void
clock_init(void)
{
}

void
fuzz_contiki_drain(void)
{
  while(process_run() > 0) {
  }
}

void
fuzz_contiki_init(void)
{
  clock_init();
  rtimer_init();
  process_init();
  process_start(&etimer_process, NULL);
  ctimer_init();
#if QUEUEBUF_ENABLED
  queuebuf_init();
#endif
  NETSTACK_RADIO.init();
  random_init();
  NETSTACK_MAC.init();
  node_id_init();
  memcpy(&uip_lladdr.addr, &linkaddr_node_addr, sizeof(uip_lladdr.addr));
  process_start(&tcpip_process, NULL);
  fuzz_contiki_drain();
}

void
fuzz_contiki_reset(void)
{
  /* Neighbor table first: its removal callbacks tear down the ds6 neighbors, routes and RPL
     neighbors while their lists are still intact. */
  nbr_table_clear();
  ctimer_init();
#if QUEUEBUF_ENABLED
  queuebuf_init();
#endif
  packetbuf_clear();
  memset(uip_buf, 0, sizeof(uip_buf));
  uip_len = 0;
  RANDOM_PRNG.seed(0x4d617968656d2121ULL);
  /* uip_init() resets the ds6 address/prefix/route/default-router state, ND, and the TCP and UDP
     connection tables; the routing init resets RPL. */
  uip_init();
  NETSTACK_ROUTING.init();
  fuzz_contiki_drain();
}

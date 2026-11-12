#ifndef FUZZ_COMMON_H_
#define FUZZ_COMMON_H_

/* Bring up the parts of the Contiki-NG native stack that main() would, minus the
   platform layer (TUN device, argv parsing, event loop). */
void fuzz_contiki_init(void);

/* Run the process scheduler until nothing is pending. */
void fuzz_contiki_drain(void);

/* Return the stack to its post-init state: neighbor, route and RPL tables, ctimers, queuebuf,
   TCP/UDP connections, and the PRNG. Called at the start of every input. */
void fuzz_contiki_reset(void);

/* sicslowpan.c / coap-transactions.c keep file-static state; these clear it. */
void fuzz_sicslowpan_reset(void);
void fuzz_coap_clear_transactions(void);

#endif /* FUZZ_COMMON_H_ */

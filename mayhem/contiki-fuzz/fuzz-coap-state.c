/* Compiled in place of coap-transactions.c (excluded in the Makefile): the unmodified upstream
   source, plus a way to drop every open transaction, whose list is file-static. */
#include "coap-transactions.c"

#include "fuzz-common.h"

void
fuzz_coap_clear_transactions(void)
{
  coap_transaction_t *t;
  while((t = (coap_transaction_t *)list_head(transactions_list)) != NULL) {
    coap_clear_transaction(t);
  }
}

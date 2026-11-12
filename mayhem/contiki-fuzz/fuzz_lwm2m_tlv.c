#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "lwm2m-tlv.h"

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  if(size == 0) {
    return -1;
  }
  /* Exact-size heap copy so reads past the end are caught by ASan. */
  uint8_t *buf = malloc(size);
  memcpy(buf, data, size);

  size_t pos = 0;
  while(pos < size) {
    lwm2m_tlv_t tlv;
    size_t used = lwm2m_tlv_read(&tlv, buf + pos, size - pos);
    if(used == 0) {
      break;
    }
    int32_t fix;
    (void)lwm2m_tlv_get_int32(&tlv);
    (void)lwm2m_tlv_float32_to_fix(&tlv, &fix, 10);
    (void)lwm2m_tlv_get_size(&tlv);
    pos += used;
  }
  free(buf);
  return 0;
}

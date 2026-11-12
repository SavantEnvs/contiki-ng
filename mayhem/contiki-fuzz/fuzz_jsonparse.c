#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "jsonparse.h"

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  if(size == 0 || size > INT32_MAX) {
    return -1;
  }
  /* The parser takes a length, not a NUL terminator: hand it an exact-size heap copy. */
  char *json = malloc(size);
  memcpy(json, data, size);

  struct jsonparse_state state;
  char value[64];
  jsonparse_setup(&state, json, (int)size);
  int type;
  while((type = jsonparse_next(&state)) != 0 && type != JSON_TYPE_ERROR) {
    switch(type) {
    case JSON_TYPE_NUMBER:
      (void)jsonparse_get_value_as_int(&state);
      (void)jsonparse_get_value_as_long(&state);
      break;
    case JSON_TYPE_STRING:
    case JSON_TYPE_PAIR_NAME:
      (void)jsonparse_copy_value(&state, value, sizeof(value));
      (void)jsonparse_strcmp_value(&state, "name");
      break;
    default:
      break;
    }
    (void)jsonparse_get_len(&state);
    (void)jsonparse_get_type(&state);
  }
  free(json);
  return 0;
}

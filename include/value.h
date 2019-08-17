#ifndef VALUE_H
#define VALUE_H

#include "stack.h"
#include "heap.h"

typedef enum {
  ROC_TEMP,
  ROC_ONSTACK,
  ROC_ONHEAP,
  ROC_NONE
} plvalue_storage_t;

typedef struct {
  jjvalue_t value;
  int16_t roc;
  int16_t type;
} plvalue_t;

plvalue_t create_onstack(stkobj_t *storage);
plvalue_t create_onheap(heapobj_t *storage);
plvalue_t create_temp();
jjvalue_t *fetch_storage(plvalue_t *obj);
result_t fetch_int(plvalue_t obj);
result_t fetch_float(plvalue_t obj);
result_t fetch_bool(plvalue_t obj);
result_t fetch_str(plvalue_t obj);
result_t fetch_list(plvalue_t obj);
result_t fetch_handle(plvalue_t obj);
plvalue_t auto_deref(plvalue_t maybe_ref);

#endif // VALUE_H

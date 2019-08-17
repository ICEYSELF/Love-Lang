#include "value.h"

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

plvalue_t create_onstack(stkobj_t *storage) {
  plvalue_t ret;
  ret.roc = ROC_ONSTACK;
  ret.type = soid2jt(storage->soid);
  ret.value.pvalue = storage;
  return ret;
}

plvalue_t create_onheap(heapobj_t *storage) {
  plvalue_t ret;
  ret.roc = ROC_ONHEAP;
  ret.type = hoid2jt(storage->oid);
  ret.value.pvalue = storage;
  return ret;
}

plvalue_t create_temp() {
  plvalue_t ret;
  ret.roc = ROC_TEMP;
  return ret;
}

jjvalue_t *fetch_storage(plvalue_t *obj) {
  switch (obj->roc) {
  case ROC_TEMP: return &(obj->value);
  case ROC_ONHEAP: {
    heapobj_t *heapobj = (heapobj_t*)(obj->value.pvalue);
    return &(heapobj->value);
  }
  case ROC_ONSTACK: {
    stkobj_t *stkobj = (stkobj_t*)(obj->value.pvalue);
    return &(stkobj->value);
  }
  }
  return NULL;
}

result_t fetch_int(plvalue_t obj) {
  jjvalue_t *storage = fetch_storage(&obj);
  if (storage == NULL) {
    return failed_result("object does not have storage");
  }

  switch (obj.type) {
  case JT_INT:
    return success_result(*storage);
  case JT_FLOAT: {
    jjvalue_t shell;
    shell.fvalue = (int)(storage->ivalue);
    return success_result(shell);
  }
  case JT_BOOL: {
    jjvalue_t shell;
    shell.ivalue = (int)(storage->bvalue);
    return success_result(shell);
  }
  case JT_STR:
    return failed_result("cannot autocast from Str to Int");
  case JT_LIST:
    return failed_result("cannot autocast from List to Int");
  case JT_UNDEFINED:
    return failed_result("cannot autocast from Nothing to Int");
  }

  UNREACHABLE;
  return failed_result("failed");
}

result_t fetch_float(plvalue_t obj) {
  jjvalue_t *storage = fetch_storage(&obj);
  if (storage == NULL) {
    return failed_result("object does not have storage");
  }

  switch (obj.type) {
  case JT_INT: {
    jjvalue_t shell;
    shell.fvalue = (double)(storage->ivalue);
    return success_result(shell);
  }
  case JT_FLOAT: return success_result(*storage);
  case JT_BOOL: {
    jjvalue_t shell;
    shell.fvalue = (float)(storage->fvalue);
    return success_result(shell);
  }
  case JT_STR:
    return failed_result("cannot autocast from Str to Float");
  case JT_LIST:
    return failed_result("cannot autocast from List to Float");
  case JT_UNDEFINED:
    return failed_result("cannot autocast from Nothing to Float");
  }

  UNREACHABLE;
  return failed_result("failed");
}

result_t fetch_bool(plvalue_t obj) {
  jjvalue_t *storage = fetch_storage(&obj);
  if (storage == NULL) {
    return failed_result("object does not have storage");
  }

  switch (obj.type) {
  case JT_INT: {
    jjvalue_t shell;
    shell.bvalue = (bool)(storage->ivalue);
    return success_result(shell);
  }
  case JT_FLOAT: {
    jjvalue_t shell;
    shell.bvalue = (int)(storage->fvalue);
    return success_result(shell);
  }
  case JT_BOOL: {
    jjvalue_t shell;
    shell.bvalue = storage->bvalue;
    return success_result(shell);
  }
  case JT_STR:
  case JT_LIST: {
    jjvalue_t shell;
    shell.bvalue = true;
    return success_result(shell);
  }
  case JT_UNDEFINED: {
    jjvalue_t shell;
    shell.bvalue = false;
    return success_result(shell);
  }
  }

  UNREACHABLE;
  return failed_result("failed");
}

result_t fetch_str(plvalue_t obj) {
  jjvalue_t *storage = fetch_storage(&obj);
  if (storage == NULL) {
    return failed_result("object does not have storage");
  }

  switch (obj.type) {
  case JT_INT: {
    char buffer[128];
    sprintf(buffer, "%" PRId64, storage->ivalue);
    jjvalue_t shell;
    shell.svalue = create_string(buffer);
    return success_result(shell);
  }
  case JT_FLOAT: {
    char buffer[128];
    sprintf(buffer, "%g", storage->fvalue);
    jjvalue_t shell;
    shell.svalue = create_string(buffer);
    return success_result(shell);
  }
  case JT_BOOL: {
    jjvalue_t shell;
    shell.svalue = storage->bvalue ? create_string("True") :
                                     create_string("False");
    return success_result(shell);
  }
  case JT_STR: return success_result(*storage);
  case JT_LIST:
    return failed_result("cannot autocast fron List to Str");
  }

  UNREACHABLE;
  return failed_result("failed");
}

result_t fetch_list(plvalue_t obj) {
  jjvalue_t *storage = fetch_storage(&obj);
  if (storage == NULL) {
    return failed_result("object does not have storage");
  }

  switch (obj.type) {
  case JT_INT:
    return failed_result("cannot autocast from Int to List");
    return failed_result("failed");  case JT_FLOAT:
    return failed_result("cannot autocast from Float to List");
  case JT_BOOL:
    return failed_result("cannot autocast from Bool to List");
  case JT_STR:
    return failed_result("cannot autocast from Str to List");
  case JT_LIST:
    return success_result(*storage);
  }

  UNREACHABLE;
  return failed_result("failed");
}

result_t fetch_handle(plvalue_t obj) {
  jjvalue_t *storage = fetch_storage(&obj);
  if (storage == NULL) {
    return failed_result("object does not have storage");
  }
  
  switch (obj.type) {
  case JT_RESHANDLE:
    return success_result(*storage);
  default:
    return failed_result("failed");
  }
}


plvalue_t auto_deref(plvalue_t maybe_ref) {
  if (maybe_ref.type != JT_REF) {
    return maybe_ref;
  }
  heapobj_t *referred =
    (heapobj_t*)(fetch_storage(&maybe_ref)->pvalue);
  return create_onheap(referred);
}

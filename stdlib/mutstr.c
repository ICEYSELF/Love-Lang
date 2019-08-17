#include "eval.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  RESOURCE_COMMON_HEAD
  char *beg, *end, *usg;
} mutstr_t;

void init_mutstr(mutstr_t *s, const char* cstr) {
  size_t len = strlen(cstr) + 1;
  s->beg = NEWN(char, len);
  s->end = s->beg + len;
  s->usg = s->beg + len;
  strcpy(s->beg, cstr);
}

char mutstr_at(mutstr_t const* s, size_t idx) {
  if (s->beg + idx >= s->end) {
    return '\0';
  }
  return s->beg[idx];
}

void mutstr_atput(mutstr_t *s, size_t idx, char ch) {
  if (s->beg + idx >= s->end) {
    eprintf("e: index %u out of range\n", idx);
  }
  s->beg[idx] = ch;
}

void mutstr_push(mutstr_t *s, char ch) {
  if (s->usg + 1 >= s->end) {
    size_t len = s->end - s->beg;
    size_t newlen = len * 2;
    char *buf = NEWN(char, newlen);
    strcpy(buf, s->beg);
    free(s->beg);
    s->beg = buf;
    s->end = buf + newlen;
    s->usg = buf + len;
  }
  *(s->usg) = ch;
  *(s->usg + 1) = '\0';
  ++(s->usg);
}

void mutstr_print(mutstr_t const* s) {
  printf("%s", s->beg);
}

void destroy_mutstr(mutstr_t* s) {
  free(s->beg);
}

void ffi_mutstr_destructor(void* p) {
  mutstr_t *s = (mutstr_t*)p;
  destroy_mutstr(s);
  free(s);
}

const char* *describe_ffi_funcs(void) {
  static const char* funcs[] = {
    "ffi_mstr_new",
    "ffi_mstr_print",
    "ffi_mstr_at",
    "ffi_mstr_atput",
    "ffi_mstr_push",  
    NULL
  };
  return funcs;
}

const char* *describe_ffi_aliases(void) {
  static const char* aliases[] = {
    "mstrnew",
    "mstrprint",
    "mstrat",
    "mstratput",
    "mstrpush",
    NULL
  };
  return aliases;
}

#define EXPECT_ARG_COUNT(ARGS, MIN, MAX, FNAME, RETV) \
  if (list_size(&ARGS) < MIN) { \
    eprintf("e: %s requires at least %d arguments\n", FNAME, MIN); \
    return RETV; \
  } \
  if (list_size(&ARGS) > MAX) { \
    eprintf("w: %s requires only %d arguments\n", FNAME, MAX); \
  }
  

plvalue_t ffi_mstr_new(list_t args) {
  plvalue_t ret = create_temp();
  ret.type = JT_UNDEFINED;
  EXPECT_ARG_COUNT(args, 1, 1, "mstrnew", ret);
  
  plvalue_t *v = (plvalue_t*)iter_deref(list_begin(&args));
  result_t r = fetch_str(*v);
  if (!r.success) {
    eprintf("e: %s\n", get_string(r.value.svalue));
  }
  
  mutstr_t *muts = NEW(mutstr_t);
  muts->destructor = ffi_mutstr_destructor;
  init_mutstr(muts, get_string(r.value.svalue));
  heapobj_t *h = heap_alloc_handle((res_base_t*)muts);
  plvalue_t ref = create_temp();
  ref.type = JT_REF;
  ref.value.pvalue = h;
  return ref;
}

plvalue_t ffi_mstr_print(list_t args) {
  plvalue_t ret = create_temp();
  ret.type = JT_UNDEFINED;
  EXPECT_ARG_COUNT(args, 1, 1, "mstrprint", ret);
  
  plvalue_t *v = (plvalue_t*)iter_deref(list_begin(&args));
  result_t r = fetch_handle(auto_deref(*v));
  if (!r.success) {
    eprintf("e: %s\n", get_string(r.value.svalue));
    return ret;
  }
  
  mutstr_t *s = (mutstr_t*)r.value.pvalue;
  mutstr_print(s);
  
  ret.type = JT_INT;
  ret.value.ivalue = 1;
  return ret;
}

plvalue_t ffi_mstr_at(list_t args) {
  (void)args;
  plvalue_t ret = create_temp();
  ret.type = JT_UNDEFINED;
  return ret;
}

plvalue_t ffi_mstr_atput(list_t args) {
  (void)args;
  plvalue_t ret = create_temp();
  ret.type = JT_UNDEFINED;
  return ret;
}

plvalue_t ffi_mstr_push(list_t args) {
  (void)args;
  plvalue_t ret = create_temp();
  ret.type = JT_UNDEFINED;
  return ret;
}

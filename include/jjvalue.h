#ifndef JJVALUE_H
#define JJVALUE_H

#include "clist.h"
#include "mstring.h"
#include "util.h"

#include <stdbool.h>
#include <stdint.h>

#define RESOURCE_COMMON_HEAD \
  void (*destructor)(void*);

typedef struct {
  RESOURCE_COMMON_HEAD;
} res_base_t;

typedef union {
  int64_t ivalue;
  double fvalue;
  bool bvalue;
  strhdl_t svalue;
  list_t lsvalue;
  void *pvalue;
} jjvalue_t;

typedef enum {
  JT_INT,
  JT_FLOAT,
  JT_BOOL,
  JT_STR,
  JT_LIST,
  JT_REF,
  JT_RESHANDLE,
  JT_UNDEFINED
} jjtype_t;

TEMPLATE(T)
typedef struct {
  jjvalue_t value;
  bool success;
} result_t;

TEMPLATE(T)
static inline result_t TP(T) failed_result(const char *error) {
  result_t TP(T) r;
  r.success = 0;
  r.value.svalue = create_string(error);
  return r;
}

TEMPLATE(T)
static inline result_t TP(T) success_result(jjvalue_t TP(T) value) {
  result_t TP(T) r;
  r.success = 1;
  r.value = value;
  return r;
}

#endif

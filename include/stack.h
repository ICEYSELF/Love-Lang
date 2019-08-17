#ifndef STACK_H
#define STACK_H

#include "clist.h"
#include "mstring.h"
#include "heap.h"

#include <stddef.h>
#include <stdint.h>

typedef enum {
  SOID_INT       = JT_INT,
  SOID_FLOAT     = JT_FLOAT,
  SOID_BOOL      = JT_BOOL,
  SOID_STR       = JT_STR,
  SOID_REF       = JT_REF,
  SOID_UNDEFINED = JT_UNDEFINED
} stkobj_id_t;

typedef struct {
  jjvalue_t value;
  strhdl_t name;
  int8_t soid;
} stkobj_t;

typedef struct {
  stkobj_t *objs_begin, *objs_end;
} stkframe_t;

typedef struct {
  stkobj_t *storage;
  int32_t stack_size;
  int32_t stack_usage;
  list_t TP(stkframe_t) frames;
} stack_t;

stkobj_id_t jt2soid(jjtype_t jt);
jjtype_t soid2jt(stkobj_id_t soid);

void init_stack(stack_t *stack);

void close_stack(stack_t *stack);

void stack_enter_frame(stack_t *stack);

void stack_exit_frame(stack_t *stack);

stkobj_t *stack_get(stack_t *stack, strhdl_t name);

void request_stack_gc(stack_t *stack);

#endif

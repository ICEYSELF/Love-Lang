#include "stack.h"

#include "config.h"
#include "util.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

stkobj_id_t jt2soid(jjtype_t jt) {
  assert(jt != JT_LIST);
  assert(jt != JT_RESHANDLE);
  return (stkobj_id_t)jt;
}

jjtype_t soid2jt(stkobj_id_t soid) {
  return (jjtype_t)soid;
}

void init_stack(stack_t *stack) {
  stack->storage = NEWN(stkobj_t, PLI_STACK_SIZE);
  stack->stack_size = PLI_STACK_SIZE;
  stack->stack_usage = 0;
  create_list(&(stack->frames), malloc, free);
}

void close_stack(stack_t *stack) {
  for (iter_t it = list_begin(&(stack->frames));
       !iter_eq(it, list_end(&(stack->frames))); 
       it = iter_next(it)) {
    free(iter_deref(it));
  }
  destroy_list(&(stack->frames));
  free(stack->storage);
}

static stkobj_t *stack_allocate(stack_t *stack, strhdl_t name) {
  if (stack->stack_usage == stack->stack_size) {
    eprintf("pilang pivm: stack overflow, "
            "with PLI_STACK_SIZE = %d\n", PLI_STACK_SIZE);
    eprintf0("note: recompile with a larger PLI_STACK_SIZE "
             "may solve this problem");
    abort();
  }

  stkobj_t *obj = stack->storage + stack->stack_usage;
  obj->name = name;

  stkframe_t *cur_frame = 
    (stkframe_t*)iter_deref(iter_prev(list_end(&(stack->frames))));
  stack->stack_usage++;
  cur_frame->objs_end++;
  return obj;
}

void stack_enter_frame(stack_t *stack) {
  stkframe_t *frame = NEW(stkframe_t);
  frame->objs_begin = stack->storage + stack->stack_usage;
  frame->objs_end = stack->storage + stack->stack_usage;
  list_push_back(&(stack->frames), frame);
}

void stack_exit_frame(stack_t *stack) {
  stkframe_t *curframe =
    (stkframe_t*)iter_deref(iter_prev(list_end(&(stack->frames))));
  stack->stack_usage -= (curframe->objs_end - curframe->objs_begin);
  list_remove(&(stack->frames), iter_prev(list_end(&(stack->frames))));
  free(curframe);
}

stkobj_t *stack_get(stack_t *stack, strhdl_t name) {
  stkframe_t *frame = 
    (stkframe_t*)iter_deref(iter_prev(list_end(&(stack->frames))));

  for (stkobj_t *obj = frame->objs_begin;
       obj != frame->objs_end;
       ++obj) {
    if (name == obj->name) {
      return obj;
    }
  }
  
  stkobj_t *obj = stack_allocate(stack, name);
  obj->soid = SOID_UNDEFINED;
  return obj;
}

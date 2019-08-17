#include "heap.h"

#include "clist.h"
#include "config.h"
#include "util.h"

#include <assert.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

heapobj_id_t jt2hoid(jjtype_t jt) {
  assert(jt != JT_REF);
  return (heapobj_id_t)jt;
}

jjtype_t hoid2jt(heapobj_id_t hoid) {
  return (jjtype_t)hoid;
}

static heap_t *glob_heap;

void destroy_object(heapobj_t *obj) {
  switch(obj->oid) {
  default:

  case HOID_INT:
  case HOID_FLOAT:
  case HOID_STR:
    break;

  case HOID_LIST:
    destroy_list(&(obj->value.lsvalue));
    break;
    
  case HOID_RESHANDLE: {
    res_base_t *res = (res_base_t*)obj->value.pvalue;
    void (*destructor)(void*) = res->destructor;
    destructor(res);
    break;
  }
  }
}

void init_heap() {
  glob_heap = NEW(heap_t);
  glob_heap->heap_storage = NEWN(heapobj_t*, PLI_HEAP_INIT_SIZE);
  for (int i = 0; i < PLI_HEAP_INIT_SIZE; i++) {
    glob_heap->heap_storage[i] = NEW(heapobj_t);
    glob_heap->heap_storage[i]->used = 0;
  }
  glob_heap->heap_cap = PLI_HEAP_INIT_SIZE;
  glob_heap->heap_usage = 0;
}

void close_heap() {
  for (size_t i = 0; i < glob_heap->heap_cap; i++) {
    if (glob_heap->heap_storage[i]->used) {
      destroy_object(glob_heap->heap_storage[i]);
    }
    free(glob_heap->heap_storage[i]);
  }
  free(glob_heap->heap_storage);
  free(glob_heap);
}

heap_t *get_glob_heap() {
  return glob_heap;
}

void set_glob_heap(heap_t *heap) {
  glob_heap = heap;
}

static void expand_heap(void) {
  size_t new_cap = glob_heap->heap_cap * 1.6;
  heapobj_t **new_heap_storage = NEWN(heapobj_t*, new_cap);
  for (size_t i = 0; i < glob_heap->heap_cap; i++) {
    new_heap_storage[i] = glob_heap->heap_storage[i];
  }
  for (size_t i = glob_heap->heap_cap; i < new_cap; i++) {
    new_heap_storage[i] = NEW(heapobj_t);
    new_heap_storage[i]->used = 0;
  }
  free(glob_heap->heap_storage);
  glob_heap->heap_storage = new_heap_storage;
  glob_heap->heap_cap = new_cap;
}

static heapobj_t *plalloc(void) {
  if (glob_heap->heap_cap == glob_heap->heap_usage) {
    // heap_request_gc();
    if (0.8 * glob_heap->heap_cap < glob_heap->heap_usage) {
      expand_heap();
    }
  }

  for (size_t i = 0; i < glob_heap->heap_cap; i++) {
    if (glob_heap->heap_storage[i]->used == 0) {
      ++glob_heap->heap_usage;
      glob_heap->heap_storage[i]->used = 1;
      return glob_heap->heap_storage[i];
    }
  }

  UNREACHABLE;
  return NULL;
}

heapobj_t *heap_alloc_int(int64_t value) {
  heapobj_t *ret = plalloc();
  ret->oid = HOID_INT;
  ret->value.ivalue = value;
  return ret;
}

heapobj_t *heap_alloc_float(double value) {
  heapobj_t *ret = plalloc();
  ret->oid = HOID_FLOAT;
  ret->value.fvalue = value;
  return ret;
}

heapobj_t *heap_alloc_bool(bool value) {
  heapobj_t *ret = plalloc();
  ret->oid = HOID_BOOL;
  ret->value.bvalue = value;
  return ret;
}

heapobj_t *heap_alloc_list(list_t list) {
  heapobj_t *ret = plalloc();
  ret->oid = HOID_LIST;
  ret->value.lsvalue = list;
  return ret;
}

heapobj_t *heap_alloc_str(int64_t str) {
  heapobj_t *ret = plalloc();
  ret->oid = HOID_STR;
  ret->value.svalue = str;
  return ret;
}

heapobj_t *heap_alloc_handle(res_base_t *res) {
  heapobj_t *ret = plalloc();
  ret->oid = HOID_RESHANDLE;
  ret->value.pvalue = res;
  return ret;
}

heapobj_t *heap_alloc_empty() {
  heapobj_t *ret = plalloc();
  ret->oid = HOID_UNDEFINED;
  return ret;
}

void gc_start() {
  for (size_t i = 0; i < glob_heap->heap_cap; i++) {
    if (glob_heap->heap_storage[i]->used) {
      glob_heap->heap_storage[i]->gcmark = GCM_BLACK;
    }
  }
}

static void gc_mark_list(heapobj_t *obj) {
  list_t list = obj->value.lsvalue;
  for (iter_t it = list_begin(&list);
       !iter_eq(it, list_end(&list));
       it = iter_next(it)) {
    gc_mark_white((heapobj_t*)iter_deref(it));
  }
}

void gc_mark_white(heapobj_t *obj) {
  obj->gcmark = GCM_WHITE;
  switch (obj->oid) {
  case HOID_LIST:
    gc_mark_list(obj);
    break;
  default:
    ;
  }
}

void gc_cleanup() {
  for (size_t i = 0; i < glob_heap->heap_cap; i++) {
    if ((glob_heap->heap_storage[i]->used) 
        && (glob_heap->heap_storage[i]->gcmark == GCM_BLACK)) {
      glob_heap->heap_storage[i]->used = 0;
      destroy_object(glob_heap->heap_storage[i]);
      --glob_heap->heap_usage;
    }
  }
}

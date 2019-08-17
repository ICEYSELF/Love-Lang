#ifndef HEAP_H
#define HEAP_H

#include "clist.h"
#include "jjvalue.h"

typedef enum {
  HOID_INT       = JT_INT,
  HOID_FLOAT     = JT_FLOAT,
  HOID_BOOL      = JT_BOOL,
  HOID_STR       = JT_STR,
  HOID_LIST      = JT_LIST,
  HOID_RESHANDLE = JT_RESHANDLE,
  HOID_UNDEFINED = JT_UNDEFINED
} heapobj_id_t;

typedef enum {
  GCM_WHITE,
  GCM_BLACK
} heap_gcmark_t;

typedef struct {
  jjvalue_t value;
  uint8_t used : 1;
  uint8_t gcmark : 1;
  uint8_t oid : 4;
  uint32_t index : 26;
} heapobj_t;

typedef struct {
  heapobj_t **heap_storage;
  size_t heap_cap;
  size_t heap_usage;
} heap_t;

heapobj_id_t jt2hoid(jjtype_t jt);
jjtype_t hoid2jt(heapobj_id_t hoid);

/// @todo FIXME do we really need these assistant functions?
heapobj_t *heap_alloc_int(int64_t value);
heapobj_t *heap_alloc_bool(bool value);
heapobj_t *heap_alloc_float(double value);
heapobj_t *heap_alloc_str(strhdl_t h_str);
heapobj_t *heap_alloc_list(list_t list);
heapobj_t *heap_alloc_handle(res_base_t *res);
heapobj_t *heap_alloc_empty();

void destroy_object(heapobj_t *obj);

void init_heap(void);
void close_heap(void);

heap_t *get_glob_heap(void);
void set_glob_heap(heap_t *heap);

void gc_start(void);
void gc_mark_white(heapobj_t *obj);
void gc_cleanup(void);

#endif

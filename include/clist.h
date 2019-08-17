#ifndef CLIST_H
#define CLIST_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

struct clistimpl_;

typedef struct clist_ {
  struct clistimpl_ *impl;
} list_t;

typedef struct citerator_ {
  void* opaque;
} iter_t;

typedef struct cciterator_ {
  const void* opaque;
} const_iter_t;

typedef void*(*allocator_t)(size_t);
typedef void(*deallocator_t)(void*);
typedef void *any_t;
typedef const void *const_any_t;

void create_list(list_t *list, allocator_t alloc,
                 deallocator_t dealloc);
void destroy_list(list_t *list);
void list_push_back(list_t *list, any_t any);
void list_push_front(list_t *list, any_t any);
void list_insert_before(list_t *list, iter_t iter, any_t any);
void list_insert_after(list_t *list, iter_t iter, any_t ant);
void list_remove(list_t *list, iter_t iter);
size_t list_size(list_t const* list);
iter_t list_begin(list_t *list);
iter_t list_end(list_t *list);
const_iter_t list_cbegin(list_t const* list);
const_iter_t list_cend(list_t const* list);

any_t iter_deref(iter_t iter);
const_any_t const_iter_deref(const_iter_t iter);

any_t list_at(list_t *list, size_t idx);
const_any_t const_list_at(list_t const* list, size_t idx);

iter_t iter_next(iter_t iter);
iter_t iter_prev(iter_t iter);

bool iter_eq(iter_t lhs, iter_t rhs);
bool const_iter_eq(const_iter_t lhs, const_iter_t rhs);

const_iter_t const_iter_next(const_iter_t iter);
const_iter_t const_iter_prev(const_iter_t iter);

#ifdef __cplusplus
}
#endif

#endif // CLIST_H

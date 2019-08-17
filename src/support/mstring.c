#include "mstring.h"

#include "util.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#if __STDC_VERSION__ >= 201112L
_Static_assert(STRING_HASHMAP_BUCKET_COUNT < 100000,
               "Too many buckets");
#endif

uint64_t BKDR_hash(const char *str) {
  uint64_t hash = 0;
  char ch = 0;
  ch = *str++;
  while (ch) {
    hash = hash * 13 + ch;
    ch = *str++;
  }
  return hash;
}

typedef struct glob_hash_map_item {
  char *value;
  struct glob_hash_map_item *next;
  strhdl_t handle;
} hash_map_item_t;

static hash_map_item_t
*glob_hash_map[STRING_HASHMAP_BUCKET_COUNT] = {0};

static hash_map_item_t *new_hash_map_item(const char *value,
                                          int64_t handle) {
  hash_map_item_t *entry = NEW(hash_map_item_t);
  entry->value = NEWN(char, strlen(value) + 1);
  strcpy(entry->value, value);
  entry->next = NULL;
  entry->handle = handle;
  return entry;
}

strhdl_t create_string(const char *str) {
  uint64_t str_hash = BKDR_hash(str);
  size_t bucket = str_hash % STRING_HASHMAP_BUCKET_COUNT;

  if (glob_hash_map[bucket]) {
    hash_map_item_t *item = glob_hash_map[bucket];

    /// one element only
    if (item->next == NULL) {
      if (!strcmp(item->value, str)) {
        return item->handle;
      } else {
        item->next = new_hash_map_item(str, bucket * 1000000 + 1);
        return item->next->handle;
      }
    }

    int n = 0;
    while (item->next != NULL) {
      if (!strcmp(item->value, str)) {
        return item->handle;
      }
      item = item->next;
      n++;
    }
    item->next = new_hash_map_item(str, bucket * 10 + n);
    return item->next->handle;
  } else {
    glob_hash_map[bucket] = new_hash_map_item(str, bucket * 10);
    return glob_hash_map[bucket]->handle;
  }
}

const char *get_string(strhdl_t hstr) {
  size_t bucket = hstr / 10;
  hash_map_item_t *item = glob_hash_map[bucket];
  while (item != NULL) {
    if (item->handle == hstr) {
      return item->value;
    }
    item = item->next;
  }

  assert(0 && "Internal error");
  return "Internal error string: "
          "there was an internal error in pilang";
}

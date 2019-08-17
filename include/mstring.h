#ifndef CMALT_MSTRING_H
#define CMALT_MSTRING_H

#include <stdint.h>

#define STRING_HASHMAP_BUCKET_COUNT 4396

#ifdef __cplusplus
extern "C" {
#endif

typedef int64_t strhdl_t;

strhdl_t create_string(const char* str);
char const* get_string(strhdl_t hstr);

#ifdef __cplusplus
}
#endif

#endif // CMALT_MSTRING_H

#ifndef DYNLOAD
#define DYNLOAD

#include "jjvalue.h"

result_t TP(void*) dyn_load_func(const char *filename,
                                 const char *funcname);

#endif // DYNLOAD

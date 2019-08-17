#include "dynload.h"
#include <dlfcn.h>

result_t TP(void*) dyn_load_func(const char *filename,
                                 const char *funcname) {
  jjvalue_t ret;
  void *handle = dlopen(filename, RTLD_NOW);
  if (handle == NULL) {
    return failed_result(dlerror());
  }
  void *func = dlsym(handle, funcname);
  if (func == NULL) {
    return failed_result(dlerror());
  }
  ret.pvalue = func;
  return success_result(ret);
}

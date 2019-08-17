#include "dynload.h"

#include <stdio.h>
#include <windows.h>

result_t TP(void*) dyn_load_func(const char *filename,
                                 const char *funcname) {
  HMODULE module = LoadLibraryA(filename);
  if (module == NULL) {
    return failed_result("e: LoadLibraryA failed");
  }
  
  FARPROC proc = GetProcAddress(module, funcname);
  if (proc == NULL) {
    return failed_result("e: GetProcAddress failed");
  }
  
  jjvalue_t ret;
  ret.pvalue = (void*)proc;
  return success_result(ret);
}

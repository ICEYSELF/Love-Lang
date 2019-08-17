#ifndef BUILTINS_H
#define BUILTINS_H

#include "value.h"
#include "clist.h"

bool is_builtin_call(strhdl_t name);
plvalue_t builtin_call(strhdl_t name, list_t args);

#endif // BUILTINS_H

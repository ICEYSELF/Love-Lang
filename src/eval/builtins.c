#include "config.h"
#include "builtins.h"
#include "dynload.h"

#include "eval.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

typedef plvalue_t (*builtin_func_t)(list_t);
typedef void (*set_host_env_t)(host_env_t);

static plvalue_t builtin_print(list_t args) {
  for (iter_t it = list_begin(&args);
       !iter_eq(it, list_end(&args));
       it = iter_next(it)) {
    plvalue_t *value = (plvalue_t*)iter_deref(it);
    *value = auto_deref(*value);
    jjvalue_t *storage = fetch_storage(value);

    switch (value->type) {
    case JT_INT:
      printf("%" PRId64, storage->ivalue); break;
    case JT_FLOAT:
      printf("%f", storage->fvalue); break;
    case JT_BOOL:
      printf("%s", storage->bvalue ? "true" : "false");
      break;
    case JT_STR:
      printf("%s", get_string(storage->svalue)); break;
    case JT_LIST:
      printf("(list)"); break;
    case JT_RESHANDLE:
      printf("(resource-handle)"); break;
    case JT_UNDEFINED:
      printf("Undefined"); break;
    default:
      printf("error");
    }
  }
  plvalue_t ret = create_temp();
  ret.type = JT_INT;
  ret.value.ivalue = list_size(&args);
  return ret;
}

static plvalue_t builtin_readint(list_t args) {
  (void)args;
  plvalue_t ret = create_temp();
  ret.type = JT_INT;
  scanf("%" PRId64, &(ret.value.ivalue));
  return ret;
}

static plvalue_t builtin_readfloat(list_t args) {
  (void)args;
  plvalue_t ret = create_temp();
  ret.type = JT_FLOAT;
  scanf("%lf", &(ret.value.fvalue));
  return ret;
}

static plvalue_t builtin_readstr(list_t args) {
  (void)args;
  static char buffer[4096];
  plvalue_t ret = create_temp();
  ret.type = JT_STR;
  scanf("%s", buffer);
  ret.value.svalue = create_string(buffer);
  return ret;
}

static plvalue_t builtin_copy_to_heap(list_t args) {
  if (list_size(&args) < 1) {
    eprintf0("e: toheap requires one argument\n");
    plvalue_t ret = create_temp();
    ret.type = JT_UNDEFINED;
    return ret;
  }

  if (list_size(&args) > 2) {
    eprintf0("w: toheap require only one argument\n");
  }

  plvalue_t *value = (plvalue_t*)iter_deref(list_begin(&args));
  *value = auto_deref(*value);
  jjvalue_t *storage = fetch_storage(value);

  if (value->roc == ROC_ONHEAP) {
    eprintf0("e: already on heap\n");
    plvalue_t ret = create_temp();
    ret.type = JT_UNDEFINED;
    return ret;
  }

  plvalue_t ref = create_temp();
  ref.type = JT_REF;
  switch (value->type) {
  case JT_INT:
    ref.value.pvalue = heap_alloc_int(storage->ivalue); break;
  case JT_FLOAT:
    ref.value.pvalue = heap_alloc_float(storage->fvalue); break;
  case JT_STR:
    ref.value.pvalue = heap_alloc_str(storage->svalue); break;
  case JT_BOOL:
    ref.value.pvalue = heap_alloc_bool(storage->bvalue); break;
  case JT_UNDEFINED:
    ref.value.pvalue = heap_alloc_empty(); break;
  default: UNREACHABLE;
  }
  return ref;
}

static plvalue_t builtin_dynload(list_t args);
static plvalue_t builtin_dynmod(list_t args);
static plvalue_t builtin_limits(list_t args);

static bool builtin_funcs_init = false;
static strhdl_t builtin_func_names[PLI_FFI_FUNC_SLOT_SIZE];
static builtin_func_t builtin_funcs[PLI_FFI_FUNC_SLOT_SIZE] = {
  builtin_print, builtin_readint, builtin_readfloat, builtin_readstr,
  builtin_copy_to_heap, NULL, NULL, NULL, NULL, NULL, NULL,
  builtin_dynload, builtin_dynmod, builtin_limits
};

static size_t func_slot_usage;
static size_t func_slot_size;

static plvalue_t builtin_dynload(list_t args) {
  plvalue_t ret = create_temp();
  ret.type = JT_UNDEFINED;
  
  if (list_size(&args) < 3) {
    eprintf0("e: dynload requires three argument\n");
    return ret;
  }
  
  if (list_size(&args) > 3) {
    eprintf0("w: dynload only requires two arguments\n");
    return ret;
  }
  
  if (func_slot_size == func_slot_usage) {
    eprintf0("e: build more farms!\n");
    return ret;
  }
  
  iter_t it = list_begin(&args);
  strhdl_t objpath, funcname, alias;
  {
    plvalue_t *objpathv = (plvalue_t*)iter_deref(it);
    result_t strresult = fetch_str(*objpathv);
    if (!strresult.success) {
      eprintf0("e: args of dynload must be strings\n");
      return ret;
    }
    objpath = strresult.value.svalue;
  }
  it = iter_next(it);
  {
    plvalue_t *funcnamev = (plvalue_t*)iter_deref(it);
    result_t strresult = fetch_str(*funcnamev);
    if (!strresult.success) {
      eprintf0("e: args of dynload must be strings\n");
      return ret;
    }
    funcname = strresult.value.svalue;
  }
  it = iter_next(it);
  {
    plvalue_t *aliasv = (plvalue_t*)iter_deref(it);
    result_t strresult = fetch_str(*aliasv);
    if (!strresult.success) {
      eprintf0("e: args of dynload must be strings\n");
      return ret;
    }
    alias = strresult.value.svalue;
  }
  
  result_t func_result = dyn_load_func(get_string(objpath),
                                       get_string(funcname));
  if (!func_result.success) {
    eprintf("e: dynload: %s\n", get_string(func_result.value.svalue));
    return ret;
  }
  
  builtin_func_t func = (builtin_func_t)(func_result.value.pvalue);
  builtin_func_names[func_slot_usage] = alias;
  builtin_funcs[func_slot_usage] = func;
  func_slot_usage++;
  ret.type = JT_STR;
  ret.value.svalue = create_string("success");
  return ret;
}

typedef const char** (*mod_desc_t)(void);
typedef void (*setup_host_env_t)(host_env_t);

static plvalue_t builtin_dynmod(list_t args) {
  plvalue_t ret = create_temp();
  ret.type = JT_UNDEFINED;
  
  if (list_size(&args) < 1) {
    eprintf0("e: dynmod requires one argument\n");
    return ret;
  }
  
  if (list_size(&args) > 1) {
    eprintf0("w: dynmod requires only one argument\n");
  }
  
  plvalue_t modnamev = *(plvalue_t*)(iter_deref(list_begin(&args)));
  result_t maybe_modname = fetch_str(modnamev);
  if (!maybe_modname.success) {
    eprintf0("e: dynmod requires string argument\n");
    return ret;
  }
  strhdl_t modname = maybe_modname.value.svalue;
  
  result_t maybe_desc_ffi_funcs = dyn_load_func(get_string(modname),
                                                "describe_ffi_funcs");
  if (!maybe_desc_ffi_funcs.success) {
    eprintf("e: %s\n", get_string(maybe_desc_ffi_funcs.value.svalue));
    return ret;
  }
  
  result_t maybe_desc_ffi_aliases = 
    dyn_load_func(get_string(modname), "describe_ffi_aliases");
  if (!maybe_desc_ffi_aliases.success) {
    eprintf("e: %s\n", get_string(maybe_desc_ffi_aliases.value.svalue));
    return ret;
  }

  mod_desc_t desc_ffi_funcs = 
    (mod_desc_t)maybe_desc_ffi_funcs.value.pvalue;
  mod_desc_t desc_ffi_aliases = 
    (mod_desc_t)maybe_desc_ffi_aliases.value.pvalue;
  
  const char **ffi_funcs = desc_ffi_funcs();
  const char **ffi_aliases = desc_ffi_aliases(); 

  for (size_t i = 0; ffi_funcs[i] != NULL; i++) {
    result_t maybe_func = dyn_load_func(get_string(modname),
                                        ffi_funcs[i]);
    if (!maybe_func.success) {
      eprintf("e: failed loading %s from module %s\n", ffi_funcs[i],
              get_string(modname));
      continue;
    }
    if (func_slot_usage == func_slot_size) {
      eprintf0("e: function slots ran out.\n");
      return ret;
    }
    builtin_func_names[func_slot_usage] = create_string(ffi_aliases[i]);
    builtin_funcs[func_slot_usage] = 
      (builtin_func_t)maybe_func.value.pvalue;
    ++func_slot_usage;
  }

  ret.type = JT_STR;
  ret.value.svalue = create_string("success");
  return ret;
}

static plvalue_t builtin_limits(list_t args) {
  (void)args;
  
  eprintf0("limits of pilang environment: \n");
  eprintf("  interpreter version: %d.%d.%d\n",
          VER_PLI_MAJOR, VER_PLI_MINOR, VER_PLI_REVISE);
  eprintf("  stack size: %d\n", PLI_STACK_SIZE);
  eprintf("  max recursion depth: %d\n", PLI_STACKFRAME_COUNT);
  eprintf("  max ffi functions: %d\n", PLI_FFI_FUNC_SLOT_SIZE - 128);
  eprintf("  build time: %s %s\n", __DATE__, __TIME__);
  
  plvalue_t ret = create_temp();
  ret.type = JT_UNDEFINED;
  return ret;
}

static void maybe_init_builtin_funcs() {
  if (!builtin_funcs_init) {
    builtin_funcs_init = true;
    func_slot_usage = 14;
    func_slot_size = PLI_FFI_FUNC_SLOT_SIZE;
    builtin_func_names[0] = create_string("print");
    builtin_func_names[1] = create_string("readint");
    builtin_func_names[2] = create_string("readfloat");
    builtin_func_names[3] = create_string("readstr");
    builtin_func_names[4] = create_string("new");
    builtin_func_names[5] = create_string("deref");
    builtin_func_names[6] = create_string("createlist");
    builtin_func_names[7] = create_string("pushback");
    builtin_func_names[8] = create_string("popback");
    builtin_func_names[9] = create_string("atput");
    builtin_func_names[10] = create_string("at");
    builtin_func_names[11] = create_string("dynload");
    builtin_func_names[12] = create_string("dynmod");
    builtin_func_names[13] = create_string("limits");
  }
}

bool is_builtin_call(strhdl_t name) {
  maybe_init_builtin_funcs();

  for (size_t i = 0; i < func_slot_usage; i++) {
    if (name == builtin_func_names[i]) {
      return true;
    }
  }
  return false;
}

plvalue_t builtin_call(strhdl_t name, list_t args) {
  maybe_init_builtin_funcs();
  
  plvalue_t ret = create_temp();
  ret.type = JT_UNDEFINED;

  for (size_t i = 0; i < func_slot_usage; i++) {
    if (name == builtin_func_names[i]) {
      if (builtin_funcs[i] == NULL) {
        eprintf("sorry, %s not implemented yet.\n", get_string(name));
        break;
      }
      ret = (builtin_funcs[i])(args);
      return ret;
    }
  }

  UNREACHABLE;
  return ret;
}

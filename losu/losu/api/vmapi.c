/*
  Losu Copyright Notice
  --------------------
    Losu is an open source programming language project under the MIT license
  that can be used for both academic and commercial purposes. There are no
  fees, no royalties, and no GNU-like restrictions. Losu qualifies
  as open source software. However, Losu is not public property, and founder
  'chen-chaochen' retains its copyright.

    Losu has been registered with the National Copyright Administration of the
  People's Republic of China, and adopts the MIT license as the copyright
  licensing contract under which the right holder conditionally licenses its
  reproduction, distribution, and modification rights to an unspecified public.

    If you use Losu, please follow the public MIT agreement or choose to enter
  into a dedicated license agreement with us.

  The MIT LICENSE is as follows
  --------------------
  Copyright  2020  chen-chaochen

    Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the “Software”), to
  deal in the Software without restriction, including without limitation the
  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
  sell copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#ifndef FILE_SRC_API_VMAPI_C
#define FILE_SRC_API_VMAPI_C
#include "losu.h"
#include "losu_bytecode.h"
#include "losu_gc.h"
#include "losu_mem.h"
#include "losu_object.h"
#include "losu_syntax.h"
#include "losu_vm.h"

#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void __vm_stacktrace(losu_vm_t vm);

char LOSU_RELEASE[] =
    "2.0.0"
    "-"
    "arm64"
    "-"
    "apple"
    "-"
    "darwin";

uint32_t LOSU_VERSION[2] = {
    (vm_version('2', '0', '0')),  // min
    (vm_version('2', '0', '0')),  // now
};

/**
 * @brief create a new losu vm
 * @param stacksize the size of stack
 * @return a new losu vm handle, NULL is failed
 */
losu_extern_t losu_vm_t vm_create(int32_t stacksize) {
  __losu_vmsig lj;
  if (stacksize < 0)
    return NULL;
  losu_vm_t vm = losu_mem_new(NULL, losu_vm);
  if (!vm)
    return NULL;
  memset(vm, 0, sizeof(losu_vm));
  vm->errjmp = &lj;
  vm->nblocks = sizeof(losu_vm);
  vm->gcDymax = 1;
  vm->gcMax = SIZE_MAX;

  int32_t jsta = setjmp(lj);
  switch (jsta) {
    case losu_signal_done: {
      vm->global = losu_objunit_new(vm, 0);
      vm->global->type = obj_unittype_global;
      __losu_vmheap_init(vm, stacksize);
      __losu_objstrpool_init(vm);
      losu_lib_init(vm);
      //   losu_syntax_lex_init(vm); // init it from syntax module
      break;
    }
    default: {
      vm_close(vm);
      return NULL;
    }
  }
  vm->errjmp = NULL;
  return vm;
}

/**
 * @brief set the argument of losu vm
 * @param vm the losu vm handle
 * @param nargs the number of argument
 * @param args the argument array
 */

;
losu_extern_t void vm_setargs(losu_vm_t vm, int32_t nargs, losu_object args[]) {
  // vm->argc = argc;
  // vm->argv = argv;
  losu_object vmarg = obj_newunit(vm, obj_unittype_array);
  obj_unitsetnum(vm, vmarg, 0, obj_newnum(vm, nargs));
  for (int32_t i = 0; i < nargs; i++)
    obj_unitsetnum(vm, vmarg, i + 1, args[i]);
  vm_setval(vm, "_", vmarg);
}

/**
 * @brief set the module of losu vm
 * @param vm the losu vm handle
 * @param nmodule the number of module
 * @param module the module array
 */
losu_extern_t void vm_setmodule(losu_vm_t vm,
                                int32_t nmodule,
                                losu_module module[]) {
  vm->nmodule = nmodule;
  vm->module = module;
}

/**
 * @brief set the pwd of losu vm
 * @param vm the losu vm handle
 * @param path the path
 */
losu_extern_t void vm_setpath(losu_vm_t vm, const char* path) {
  vm->path = path;
}

/**
 * @brief throw a error message
 * @param vm the losu vm handle
 * @param msg the error message
 * @param ... the error message list
 */
losu_extern_t void vm_error(losu_vm_t vm, const char* msg, ...) {
  char tmp[128] = {0};
  va_list ap;
  va_start(ap, msg);
  vsnprintf(tmp, sizeof(tmp), msg, ap);
  fprintf(stderr, "losu v%s\n\truntime error\n\t%s\n\tat line ", LOSU_RELEASE,
          tmp);
  __vm_stacktrace(vm);
  va_end(ap);
  fprintf(stderr, "\tof %s\n", vm->name != NULL ? vm->name : "<unknown>");
  losu_sigmsg_throw(vm, losu_signal_error);
}

/**
 * @brief throw a warning message
 * @param vm the losu vm handle
 * @param msg the warning message
 * @param ... the warning message list
 */
losu_extern_t void vm_warning(losu_vm_t vm, const char* msg, ...) {
  char tmp[128] = {0};
  va_list ap;
  va_start(ap, msg);
  vsnprintf(tmp, sizeof(tmp), msg, ap);
  fprintf(
      stderr,
      "<span style='color:yellow'>losu v%s\n<span "
      "style='color:yellow'>\truntime warning</span>\n<span "
      "style='color:yellow'>\t%s</span>\n\t<span style='color:yellow'>at line ",
      LOSU_RELEASE, tmp);
  __vm_stacktrace(vm);
  va_end(ap);
  fprintf(stderr, "\tof %s\n</span>",
          vm->name != NULL ? vm->name : "<unknown>");
}

/**
 * @brief panic a error message
 * @param vm the losu vm handle
 * @param msg the error message
 * @param ... the error message list
 */
losu_extern_t void vm_panic(losu_vm_t vm, const char* msg, ...) {
  char tmp[128] = {0};
  va_list ap;
  va_start(ap, msg);
  vsnprintf(tmp, sizeof(tmp), msg, ap);
  fprintf(stderr, "losu v%s\nruntime panic\n%s\nat line ", LOSU_RELEASE, tmp);
  __vm_stacktrace(vm);
  va_end(ap);
  fprintf(stderr, "of %s\n", vm->name != NULL ? vm->name : "<unknown>");
  losu_sigmsg_throw(vm, losu_signal_kill);
}

/**
 * @brief do a file (source or bytecode)
 * @param vm the losu vm handle
 * @param path the path of file
 * @return 0 is success, not is failed
 */
losu_extern_t int32_t vm_dofile(losu_vm_t vm, const char* path) {
  int32_t i = vm_loadfile(vm, path, path);
  if (!i)
    return vm_execute(vm, 0, -1, path);
  return i;
}

/**
 * @brief do a string
 * @param vm  the losu vm handle
 * @param str the string
 * @return 0 is success, not is failed
 */
losu_extern_t int32_t vm_dostring(losu_vm_t vm, const char* str) {
  char tmp[64] = {0};
  snprintf(tmp, sizeof(tmp), "%.48s ... ", str);
  for (size_t i = 0; i < strlen(tmp); i++)
    if (tmp[i] == '\n' || tmp[i] == '\r')
      tmp[i] = '\t';
  int32_t i = vm_loadbyte(vm, str, strlen(str), tmp);
  if (!i)
    return vm_execute(vm, 0, 0, tmp);
  return i;
}

/**
 * @brief do a bytecode or source
 * @param vm the losu vm handle
 * @param byte the bytecode or source
 * @param len the length of bytecode or source
 * @return 0 is success, not is failed
 */
losu_extern_t int32_t vm_dobyte(losu_vm_t vm, const char* byte, size_t len) {
  int32_t i = vm_loadbyte(vm, byte, len, "byte array");
  if (!i)
    return vm_execute(vm, 0, 0, "byte array");
  return i;
}

/**
 * @brief load a file or bytecode
 * @param vm the losu vm handle
 * @param path the path of file
 * @param name the name of file
 * @return 0 is success, not is failed
 */
losu_extern_t int32_t vm_loadfile(losu_vm_t vm,
                                  const char* path,
                                  const char* name) {
  return losu_syntax_io_loadfile(vm, path, name);
}

/**
 * @brief load a bytecode
 * @param vm the losu vm handle
 * @param byte the bytecode
 * @param len the length of bytecode
 * @param name the name of bytecode
 * @return 0 is success, not is failed
 */
losu_extern_t int32_t vm_loadbyte(losu_vm_t vm,
                                  const char* byte,
                                  size_t len,
                                  const char* name) {
  return losu_syntax_io_loadstring(vm, byte, len, name);
}

/**
 * @brief execute a bytecode or source
 * @param vm the losu vm handle
 * @param narg the number of argument
 * @param nres the number of result
 * @param name the name of bytecode or source
 * @return 0 is success, not is failed
 */
losu_extern_t int32_t vm_execute(losu_vm_t vm,
                                 int32_t narg,
                                 int32_t nres,
                                 const char* name) {
  losu_object_t func = (vm->top - 1) - narg;
  __losu_vmsig_t oldjmp = vm->errjmp;
  const char* oldname = vm->name;
  losu_ctype_bool oldyield = vm->yield;

  __losu_vmsig jmp;
  vm->errjmp = &jmp;
  vm->name = name;
  vm->yield = 0;

  int32_t jsta = setjmp(*vm->errjmp);
  switch (jsta) {
    case losu_signal_done: {
      __losu_vmheap_rawcall(vm, func, nres);
      break;
    }
    case losu_signal_error: {
      break;
    }
    case losu_signal_kill: {
      vm->errjmp = oldjmp;
      losu_sigmsg_throw(vm, losu_signal_kill);
      break;
    }
    default: {
      break;
    }
  }
  vm->yield = oldyield;

  vm->top = func;
  if (vm->stack == vm->mstack)
    vm->mtop = vm->top;
  vm->errjmp = oldjmp;
  vm->name = oldname;
  return jsta;
}

/**
 * @brief get a value
 * @param vm the losu vm handle
 * @param name the name of value
 * @return losu_object_t the value
 */
losu_extern_t losu_object_t vm_getval(losu_vm_t vm, const char* name) {
  return (losu_object_t)losu_objunit_getstr(vm, vm->global,
                                            losu_objstring_new(vm, name));
}

/**
 * @brief set a value
 * @param vm the losu vm handle
 * @param name the name of value
 * @param val the value
 */
losu_extern_t void vm_setval(losu_vm_t vm, const char* name, losu_object val) {
  *losu_objunit_setstr(vm, vm->global, losu_objstring_new(vm, name)) = val;
}

/**
 * @brief await a coroutine
 * @param vm the losu vm handle
 * @param coro the coroutine
 * @return -1 is failed,  >= 0 is signal
 */
losu_extern_t int32_t vm_await(losu_vm_t vm, losu_object_coroutine_t coro) {
  if (!coro)
    return -1;

  losu_object_t oldtop = vm->top;
  losu_object_t oldstack = vm->stack;
  losu_object_t oldstackmax = vm->stackmax;
  losu_ctype_bool oldyield = vm->yield;

  __losu_vmsig corojmp;
  __losu_vmsig_t oldjmp = vm->errjmp;
  vm->errjmp = &corojmp;

  /* change stack size */
  vm->stack = coro->stack;
  vm->top = coro->top;
  vm->stackmax = vm->stack + coro->nstack - 1;
  vm->yield = 1;

  int32_t jid = setjmp(*vm->errjmp);
  switch (jid) {
    case losu_signal_done: {
      if (coro->sta == losu_objcoro_stateREADY) {
        coro->sta = losu_objcoro_stateRUN;
        __losu_vmheap_procall(vm, vm->stack, 0);
        coro->top = vm->top;
        coro->sta = losu_objcoro_stateEND;
        break;
      }
      if (coro->sta == losu_objcoro_stateYIELD) {
        losu_object_t recall = NULL;
        losu_object_t tmp = NULL;
        /* recall */
        for (losu_object_t o = vm->stack; o < vm->top; o++) {
          if (ovtype(o) == losu_object_type_context) {
            // printf("context:%p ,base %p\n", o,ovcall(o)->base);
            if (recall == NULL) {
              recall = tmp = o;
            } else {
              ovcall(tmp)->nextobj = o;
              tmp = o;
            }
            ovcall(tmp)->nextobj = NULL;
          }
        }
        coro->sta = losu_objcoro_stateRUN;
        __losu_vmcore_exec(vm, NULL, recall);
        coro->sta = losu_objcoro_stateEND;
        break;
      }
      if (coro->sta == losu_objcoro_stateEND)
        jid = -1;
      break;
    }
    case losu_signal_error: {
      coro->sta = losu_objcoro_stateEND;
      coro->top = vm->top;
      break;
    }
    case losu_signal_yield: {
      coro->sta = losu_objcoro_stateYIELD;
      coro->top = vm->top;
      break;
    }
    case losu_signal_kill: {
      vm->errjmp = oldjmp;
      coro->sta = losu_objcoro_stateEND;
      losu_sigmsg_throw(vm, losu_signal_kill);
      break;
    }
    default: {
      break;
    }
  }

  vm->yield = oldyield;
  vm->stack = oldstack;
  vm->top = oldtop;
  vm->errjmp = oldjmp;
  vm->stackmax = oldstackmax;
  return jid;
}

/**
 * @brief close a losu vm
 * @param vm the losu vm handle
 */
losu_extern_t void vm_close(losu_vm_t vm) {
  losu_gc_clean(vm, 1);
  __losu_objstrpool_deinit(vm);
  if (vm->mstack) {
    losu_mem_free(vm, vm->mstack);
    vm->nblocks -= (vm->mstackmax - vm->stack + 1) * sizeof(losu_object);
  }
  if (vm->bufftmp) {
    losu_mem_free(vm, vm->bufftmp);
    vm->nbufftmp -= vm->nbufftmp * sizeof(char);
  }
  losu_mem_free(NULL, vm);
}

static int32_t __vm_tryline(int32_t* lineinfo, int32_t pc);
static int32_t __vm_currentpc(losu_object_t obj);
static void __vm_stacktrace(losu_vm_t vm) {
  losu_object_t obj = vm->top - 1;
  while (1) {
    while (1) {
      if (obj < vm->stack) {
        fprintf(stderr, "\n");
        return;
      }
      if ((obj && ovtype(obj) == losu_object_type_context &&
           ovcontext(obj)->func && ovcontext(obj)->func->isC == 0))
        break;
      obj--;
    }
    int32_t line = __vm_tryline(ovcontext(obj)->func->func.sdef->lineinfo,
                                __vm_currentpc(obj));
    fprintf(stderr, "%d", line);
    if (line == -1) {
      fprintf(stderr, "\n");
      return;
    }
    obj--;
    if (obj < vm->stack) {
      fprintf(stderr, "\n");
      return;
    }
    fprintf(stderr, ", ");
  }
}

static int32_t __vm_tryline(int32_t* lineinfo, int32_t pc) {
  int32_t refi = 0;
  int32_t refline = 1;
  if (lineinfo == NULL || pc == -1)
    return -1;
  if (lineinfo[refi] < 0)
    refline += -lineinfo[refi++];
  while (lineinfo[refi] > pc) {
    refline--;
    refi--;
    if (lineinfo[refi] < 0)
      refline -= -lineinfo[refi++];
  }

  while (1) {
    int32_t nextline = refline + 1;
    int32_t nexref = refi + 1;
    if (lineinfo[nexref] < 0)
      nextline += -lineinfo[nexref++];
    if (lineinfo[nexref] > pc)
      break;
    refline = nextline;
    refi = nexref;
  }

  return refline;
}
static int32_t __vm_currentpc(losu_object_t obj) {
  if (ovcontext(obj)->pc_t)
    return ((*(ovcall(obj)->pc_t)) - ovcall(obj)->func->func.sdef->code) - 1;
  else
    return -1;
}

#endif

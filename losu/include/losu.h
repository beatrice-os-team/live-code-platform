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

#ifndef FILE_SRC_LOSU_H
#define FILE_SRC_LOSU_H

/**
 * include head file segment
 * */
#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>

// /**
//  * cfg segment
//  * */
// #include "cfg/losu.cfg"
// #ifndef FILE_CFG_LOSU_ALL
// #error "file 'cfg/losu.cfg' not found "
// #endif

/**
 * losu ctype segment
 * */
#if 1

/* losu_extern_t */
#ifndef losu_extern_t
#ifdef __cplusplus
#define losu_extern_t extern "C"
#else
#define losu_extern_t extern
#endif
#endif
/* losu_ctype_xxx */
typedef double losu_ctype_number;
typedef uint8_t losu_ctype_bool;
typedef size_t losu_ctype_size_t;
typedef int64_t losu_ctype_ssize_t;
typedef uint32_t losu_ctype_hash_t;
typedef uint32_t losu_ctype_vmins_t;

/* losu type */
typedef enum losu_object_type {
  losu_object_type_false,     /* null */
  losu_object_type_true,      /* true */
  losu_object_type_number,    /* number */
  losu_object_type_string,    /* string */
  losu_object_type_function,  /* function */
  losu_object_type_unit,      /* unit */
  losu_object_type_pointer,   /* pointer */
  losu_object_type_coroutine, /* coroutine */
  losu_object_type_context,   /* context */
  losu_object_type_any,       /* unknown */
} losu_object_type;

#endif

/**
 * losu signal segment
 *    api: losu_signal.h
 * */
#if 1

/* signal */
typedef uint8_t losu_signal_t;
typedef uint8_t losu_sigmsg_t;
#define losu_signal_done ((losu_signal_t)0)  /* done  */
#define losu_signal_error ((losu_signal_t)1) /* throw error */
#define losu_signal_yield ((losu_signal_t)2) /* yield signal*/
#define losu_signal_kill ((losu_signal_t)3)  /* kill over */

#endif

/**
 * losu vm segment
 *    with: pacakeg & handle
 *    api: losu_vm.h
 */
#if 1

typedef struct losu_vm {
  /* vm-name */
  const char* name;
  /* stack */
  struct losu_object *stack, *top, *stackmax;    /* now stack */
  struct losu_object *mstack, *mtop, *mstackmax; /*  main stack */

  /* signal */
  jmp_buf* errjmp;
  /* data segment */
  struct losu_object_scode* inspool;      /* script pool */
  struct losu_object_function* funcpool;  /* function pool */
  struct losu_object_hash* hashpool;      /* hash pool */
  struct losu_object_context* callpool;   /* callinfo pool, for  coroutine */
  struct losu_object_coroutine* coropool; /*  coroutine pool */
  struct __losu_vm_strpool {
    losu_ctype_hash_t size;
    losu_ctype_hash_t nsize;
    struct losu_object_string** strobj; /* strobj[] */
  } strpool;                            /* string pool */
  struct losu_object_hash* global;      /* global & root global */
  /* gc */
  losu_ctype_size_t gcDymax;
  losu_ctype_size_t nblocks;
  losu_ctype_size_t gcMax;
  losu_ctype_size_t gcHook;
  /* buff */
  unsigned char* bufftmp;
  losu_ctype_size_t nbufftmp;
  /* path */
  const char* path;
  /* module */
  struct losu_module* module;
  uint32_t nmodule;
  /* yield */
  losu_ctype_bool yield;
  /* gc */
  losu_ctype_bool gc;
} losu_vm, *losu_vm_t;

typedef jmp_buf __losu_vmsig, *__losu_vmsig_t;

typedef struct __losu_vm_strpool __losu_vm_strpool, *__losu_vm_strpool_t;

typedef struct losu_vm_vaarg {
  int32_t argc;
  struct losu_object* argv; /* obj[] */
  struct losu_vm* vm;
  int32_t size;
} losu_vm_vaarg, *losu_vm_vaarg_t;

typedef int32_t (*losu_vmapi_t)(losu_vm_t vm,
                                int32_t argc,
                                struct losu_object* argv);

#endif

/**
 * losu object type segment
 * */
#if 1

/* losu object */
typedef struct losu_object {
  uint8_t type;
  union {
    struct losu_object_string* str;
    struct losu_object_function* func;
    struct losu_object_hash* hash;
    struct losu_object_context* context;
    struct losu_object_coroutine* coro;
    losu_ctype_number num;
    void* ptr;
  } value;
} losu_object, *losu_object_t;

/* string */
typedef struct losu_object_string {
  losu_ctype_hash_t hash;
  int32_t cstidx;
  size_t len;
  struct losu_object_string* next;
  int16_t marked;
  char str[1]; /* char* */
} losu_object_string, *losu_object_string_t;

/* func */
typedef struct losu_object_function /* need: losu_object_scode */
{
  union func {
    losu_vmapi_t capi;
    struct losu_object_scode* sdef;
  } func;
  struct losu_object_function* next;
  struct losu_object_function* mark;
  losu_ctype_bool isC;
  int32_t nclosure;
  struct losu_object closure[1]; /* closure */
} losu_object_function, *losu_object_function_t;

typedef struct losu_object_scode {
  /* local segment */
  losu_ctype_number* lcnum;
  struct losu_object_string** lcstr;
  struct losu_object_scode** lcscode;
  losu_ctype_vmins_t* code;
  struct losu_objscode_locvar* localvar;
  int32_t* lineinfo;
  uint32_t nlcnum, nlcstr, nlcscode, ncode, nlocalvar, nlineinfo;

  int16_t narg;
  losu_ctype_bool isVarg;
  int16_t maxstacksize;
  struct losu_object_scode* next;
  losu_ctype_bool marked;

} losu_object_scode, *losu_object_scode_t;

typedef struct losu_objscode_locvar {
  struct losu_object_string* name;
  int32_t startpc;
  int32_t endpc;
} losu_objscode_locvar, *losu_objscode_locvar_t;

/* hash */
typedef struct losu_object_hash /* need: losu_hash_node */
{
  int32_t size;
  uint8_t type;  // unknown array object class
  struct losu_hash_node* node;
  struct losu_hash_node* free;
  struct losu_object_hash* next;
  struct losu_object_hash* mark;
} losu_object_hash, *losu_object_hash_t;

typedef struct losu_hash_node {
  losu_object key;
  losu_object value;
  struct losu_hash_node* next;
} losu_hash_node, *losu_hash_node_t;

/* context */
typedef struct losu_object_context {
  struct losu_object_function* func;
  losu_ctype_vmins_t *pc, **pc_t;
  struct losu_object* base;
  int32_t nres;
  void (*inlinecall)(struct losu_vm* vm,
                     struct losu_object* func,
                     int32_t nres);
  struct losu_object* nextobj; /* context link */
  struct losu_object_context *pre, *next;
  struct losu_object assert; /* assert back */
  losu_ctype_bool marked;
} losu_object_context, *losu_object_context_t;

/* coroutine */
typedef uint8_t losu_objcoro_state_t;

typedef struct losu_object_coroutine {
  int32_t nstack;
  losu_object* stack;
  losu_object_t top;
  losu_objcoro_state_t sta;
  struct losu_object_coroutine* next;
} losu_object_coroutine, *losu_object_coroutine_t;

/* package & handle */
typedef struct losu_module {
  const char* name;
  losu_object (*load)(losu_vm_t vm);
  losu_ctype_bool isloaded;
} losu_module, *losu_module_t;

#endif

#if 1
/* release x.x.x-os-cpu */
#define LOSU_MAGIC 0x36363655534f4c00
losu_extern_t char LOSU_RELEASE[];
losu_extern_t losu_module LOSU_MODULE[];
losu_extern_t uint32_t LOSU_NMODULE;
losu_extern_t uint32_t LOSU_VERSION[2];
losu_extern_t void losu_lib_init(losu_vm_t vm);

/* vm apis */
#if 1
#define vm_version(a, b, c)                                                  \
  ((uint32_t)((((uint32_t)(('0' <= a && a <= '9') ? a - '0' : a - 'a' + 10)) \
               << 16) |                                                      \
              (((uint32_t)(('0' <= b && b <= '9') ? b - '0' : b - 'a' + 10)) \
               << 8) |                                                       \
              (((uint32_t)(('0' <= c && c <= '9') ? c - '0'                  \
                                                  : c - 'a' + 10)))))

losu_extern_t losu_vm_t vm_create(int32_t stacksize);
losu_extern_t void vm_setargs(losu_vm_t vm, int32_t nargs, losu_object args[]);
losu_extern_t void vm_setmodule(losu_vm_t vm,
                                int32_t nmodule,
                                losu_module module[]);
losu_extern_t void vm_setpath(losu_vm_t vm, const char* path);

losu_extern_t void vm_error(losu_vm_t vm, const char* msg, ...);
losu_extern_t void vm_warning(losu_vm_t vm, const char* msg, ...);
losu_extern_t void vm_panic(losu_vm_t vm, const char* msg, ...);

losu_extern_t int32_t vm_dofile(losu_vm_t vm, const char* path);
losu_extern_t int32_t vm_dostring(losu_vm_t vm, const char* str);
losu_extern_t int32_t vm_dobyte(losu_vm_t vm, const char* byte, size_t len);

losu_extern_t int32_t vm_loadfile(losu_vm_t vm,
                                  const char* path,
                                  const char* name);
losu_extern_t int32_t vm_loadbyte(losu_vm_t vm,
                                  const char* byte,
                                  size_t len,
                                  const char* name);
losu_extern_t int32_t vm_execute(losu_vm_t vm,
                                 int32_t narg,
                                 int32_t nres,
                                 const char* name);

losu_extern_t losu_object_t vm_getval(losu_vm_t vm, const char* name);
losu_extern_t void vm_setval(losu_vm_t vm, const char* name, losu_object val);

losu_extern_t int32_t vm_await(losu_vm_t vm, losu_object_coroutine_t coro);

losu_extern_t void vm_close(losu_vm_t vm);
#endif

/* gc apis */
#if 1
losu_extern_t void gc_setthreshold(losu_vm_t vm, losu_ctype_size_t threshold);

losu_extern_t losu_ctype_size_t gc_getthreshold(losu_vm_t vm);

losu_extern_t losu_ctype_size_t gc_getmemnow(losu_vm_t vm);

losu_extern_t losu_ctype_size_t gc_getmemmax(losu_vm_t vm);

losu_extern_t losu_ctype_bool gc_collect(losu_vm_t vm);

#endif

/* stack apis */
#if 1
losu_extern_t void stack_push(losu_vm_t vm, losu_object o);

losu_extern_t void stack_pop(losu_vm_t vm, int32_t i);

losu_extern_t losu_ctype_bool stack_call(losu_vm_t vm,
                                         int32_t narg,
                                         int32_t nres);

losu_extern_t void stack_rawcall(losu_vm_t vm, int32_t narg, int32_t nres);

#endif

/* obj apis */
#if 1
losu_extern_t int32_t obj_type(losu_vm_t vm, losu_object_t obj);

losu_extern_t const char* obj_typestr(losu_vm_t vm, losu_object_t obj);

/* null */
#if 1
losu_extern_t losu_object obj_newnull(losu_vm_t vm);

#endif

/* true */
#if 1
losu_extern_t losu_object obj_newtrue(losu_vm_t vm);

#endif

/* number */
#if 1
losu_extern_t losu_object obj_newnum(losu_vm_t vm, losu_ctype_number num);

losu_extern_t losu_ctype_number obj_tonum(losu_vm_t vm, losu_object_t obj);

losu_extern_t losu_ctype_number obj_getnum(losu_vm_t vm, losu_object_t obj);

#endif

/* string */
#if 1
losu_extern_t losu_object obj_newstr(losu_vm_t vm, const char* str);

losu_extern_t losu_object obj_newstrlen(losu_vm_t vm,
                                        const char* str,
                                        uint32_t len);

losu_extern_t const char* obj_tostr(losu_vm_t vm, losu_object_t obj);

losu_extern_t const char* obj_getstr(losu_vm_t vm, losu_object_t obj);

losu_extern_t size_t obj_getstrlen(losu_vm_t vm, losu_object_t obj);

#endif

/* func */
#if 1
losu_extern_t losu_object obj_newfunc(losu_vm_t vm, losu_vmapi_t func);

losu_extern_t losu_vmapi_t obj_getfunc(losu_vm_t vm, losu_object_t obj);

#endif

/* unit */
#if 1

typedef enum obj_unittype {
  obj_unittype_unknown,
  obj_unittype_array,
  obj_unittype_object,
  obj_unittype_class,
  obj_unittype_module,
  obj_unittype_global,
} obj_unittype;

losu_extern_t losu_object obj_newunit(losu_vm_t vm, uint8_t type);

losu_extern_t const losu_object_t obj_unitindex(losu_vm_t vm,
                                                losu_object unit,
                                                losu_object key);
losu_extern_t const losu_object_t obj_unitindexnum(losu_vm_t vm,
                                                   losu_object unit,
                                                   losu_ctype_number key);
losu_extern_t const losu_object_t obj_unitindexstr(losu_vm_t vm,
                                                   losu_object unit,
                                                   const char* key);

losu_extern_t losu_ctype_bool obj_unitset(losu_vm_t vm,
                                          losu_object unit,
                                          losu_object key,
                                          losu_object val);
losu_extern_t losu_ctype_bool obj_unitsetnum(losu_vm_t vm,
                                             losu_object unit,
                                             losu_ctype_number key,
                                             losu_object val);
losu_extern_t losu_ctype_bool obj_unitsetstr(losu_vm_t vm,
                                             losu_object unit,
                                             const char* key,
                                             losu_object val);

losu_extern_t losu_hash_node_t obj_unititer(losu_vm_t vm,
                                            losu_object unit,
                                            losu_hash_node_t node);
losu_extern_t losu_object obj_unititerkey(losu_vm_t vm, losu_hash_node_t node);
losu_extern_t losu_object obj_unititerval(losu_vm_t vm, losu_hash_node_t node);

#endif

/* pointer */
#if 1
losu_extern_t losu_object obj_newptr(losu_vm_t vm, void* ptr);

losu_extern_t void* obj_toptr(losu_vm_t vm, losu_object_t obj);

losu_extern_t void* obj_getptr(losu_vm_t vm, losu_object_t obj);

#endif

#endif

/* arg apis */
#if 1
losu_extern_t losu_vm_vaarg_t arg_start(losu_vm_t vm, int32_t size);

losu_extern_t losu_ctype_bool arg_add(losu_vm_vaarg_t valist,
                                      int32_t type,
                                      ...);

losu_extern_t losu_ctype_bool arg_del(losu_vm_vaarg_t valist, int32_t n);

losu_extern_t int32_t arg_tostack(losu_vm_vaarg_t valist);

#endif

#endif

#endif

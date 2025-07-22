#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "losu.h"
#include "losu_dump.h"
// #include "losu_gc.h"
// /* // // int32_t lousapi_abs(losu_vm_t vm, int32_t argc, losu_object argv[]) {
// // //   losu_vm_vaarg_t args = arg_start(vm, 1);
// // //   arg_add(args, losu_object_type_number, fabs(obj_tonum(vm, &argv[0])));
// // //   return arg_tostack(args);
// // // }

// // // void test(int* a) {
// // //   printf("%p\n", &a[0]);
// // //   printf("%p\n", &a[1]);
// // //   printf("%p\n", &a[2]);
// // //   printf("%p\n", &a[3]);
// // // }

// // int32_t test(losu_vm_t vm, int32_t argc, losu_object argv[]) {
// //   // vm_warning(vm, "warning test");
// //   // vm_error(vm, "error test");
// //   // printf("[argc %d],[type]%d\n",argc, argv[0].type);
// //   for (int i = 0; i < argc; i++)
// //     printf("%s", obj_tostr(vm, &argv[i]));
// //   printf("\n");
// //   losu_vm_vaarg_t a = arg_start(vm, 1);
// //   arg_add(a, losu_object_type_number, argc);
// //   return arg_tostack(a);

// //   // printf("%s\n", obj_tostr(vm, &argv[0]));
// //   // losu_vm_vaarg_t a = arg_start(vm, 1);
// //   // arg_add(a, losu_object_type_number, (losu_ctype_number)1);
// //   // arg_del(a, 1);
// //   // arg_add(a, losu_object_type_string, (const char*)"hello world");
// //   // return arg_tostack(a);
// // }

// // int32_t test2(losu_vm_t vm, int32_t argc, losu_object argv[]) {
// //   // vm_warning(vm, "warning test");
// //   // vm_error(vm, "error test");
// //   losu_vm_vaarg_t a = arg_start(vm, 1);
// //   arg_add(a, losu_object_type_number,
// //           (losu_ctype_number)((clock_t)(((double)clock() / CLOCKS_PER_SEC) *
// //                                         1000)));

// //   return arg_tostack(a);
// // }
// // int32_t test3(losu_vm_t vm, int32_t argc, losu_object argv[]) {
// //   int n = obj_tonum(vm, &argv[0]);
// //   n = n > 0 ? n : -n;
// //   // stack_push(vm,obj_newnum(vm,n));
// //   // return 1;
// //   losu_vm_vaarg_t va = arg_start(vm, 4);
// //   arg_add(va, losu_object_type_number, (losu_ctype_number)n);
// //   return arg_tostack(va);
// // } */

// int32_t hello(losu_vm_t vm, int32_t argc, struct losu_object* argv) {
//   printf("hello\n");
//   return 0;
// }
// losu_object test_load(losu_vm_t vm) {
//   losu_object m = obj_newunit(vm, 0);
//   // obj_unitsetstr(vm, m, "text", obj_newstr(vm, "hello world"));
//   // obj_unitsetstr(vm, m, "hello", obj_newfunc(vm, hello));
//   return m;
// }

// losu_module mod[] = {
//     {"test", test_load, 0},
// };
int main() {
  losu_vm_t vm = vm_create(1024);
  vm_setargs(vm, 0, NULL);
  // printf("vm: %p\n", &vm);
  // losu_lib_init(vm);
  // vm_setmodule(vm, 1, mod);
  // vm_setval(vm, "print", obj_newfunc(vm, test));
  // vm_setval(vm, "time", obj_newfunc(vm, test2));
  // vm_setval(vm, "assert", obj_newfunc(vm, test3));
  // vm_setval(vm, "global",
  //           (losu_object){
  //               .type = losu_object_type_unit,
  //               .value.hash = vm->global,
  //           });
  vm_setpath(vm, "./");
  vm_setmodule(vm, LOSU_NMODULE, LOSU_MODULE);
  gc_setthreshold(vm, 0);
  // vm_dostring(vm, "def fib(i):\n\tif i<3:\n\t\treturn 1\n\treturn fib(i-1) +
  // fib(i-2)\nfib(40)");
  losu_dumpfile("demo.els", "demo.els.hex");
  if (vm_dofile(vm, "./demo.els.hex") == 0) {
    losu_ctype_bool b = 1;
    while (b) {
      b = 0;
      for (losu_object_coroutine_t coro = vm->coropool; coro;
           coro = coro->next) {
        if (vm_await(vm, coro) != -1)
          b = 1;
      }
    }
    gc_collect(vm);
    // while (vm->coropool) {
    //   for (losu_object_coroutine_t coro = vm->coropool; coro;
    //        coro = coro->next) {
    //         vm_await(vm,coro);
    //     // printf("resume\n");
    //     // printf("await:%d,", vm_await(vm, coro));
    //     // printf("sta=%d\n", coro->sta);
    //   }
    //   // printf("gc start %p,%p\n", vm->stack, vm->top);
    //   losu_gc_newtask(vm);
    // }
  }
  printf("--------------------------------\n");
  printf("mem max: %.8g KB\n", (double)gc_getmemmax(vm) / 1024);
  printf("mem now: %.8g KB\n", (double)gc_getmemnow(vm) / 1024);
  vm_close(vm);
  return 0;
}


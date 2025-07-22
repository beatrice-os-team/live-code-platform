#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>

#include "losu.h"
EMSCRIPTEN_KEEPALIVE void run(const char* input) {
  losu_vm_t vm = vm_create(1024);
  if (!vm) {
    fprintf(stderr, "Failed to create Losu VM\n");
    return;
  }
  vm_setargs(vm, 0, NULL);
  vm_setmodule(vm, LOSU_NMODULE, LOSU_MODULE);
  if (vm_dostring(vm, input) == 0) {
    losu_ctype_bool b = 1;
    while (b) {
      b = 0;
      for (losu_object_coroutine_t coro = vm->coropool; coro;
           coro = coro->next) {
        if (vm_await(vm, coro) != -1)
          b = 1;
      }
    }
    gc_setthreshold(vm, 0);
    gc_collect(vm);
    printf("--------------------------------\n");
    printf("mem max: %.8g KB\n", (double)gc_getmemmax(vm) / 1024);
    printf("mem now: %.8g KB\n", (double)gc_getmemnow(vm) / 1024);
    printf("运行结束\n");
  } else {
    fprintf(stderr, "运行错误\n");
  }
  vm_close(vm);
}
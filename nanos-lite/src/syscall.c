#include "syscall.h"
#include "am.h"
#include <common.h>

static inline void sys_yield(Context *c) {
  yield();
  c->GPRx = 0;
}

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

  switch (a[0]) {
  case SYS_yield:
    sys_yield(c);
    break;
  case SYS_exit:
    halt(0);
    break;
  default:
    panic("Unhandled syscall ID = %d", a[0]);
  }
}

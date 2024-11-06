#include "syscall.h"
#include "am.h"
#include <common.h>

typedef signed long ssize_t;

static inline void sys_yield(Context *c) {
  yield();
  c->GPRx = 0;
}

static inline ssize_t sys_write(int fd, const void *buf, size_t count) {
  switch (fd) {
  case 1:
  case 2:
    for (size_t i = 0; i < count; i++) {
      putch(((char *)buf)[i]);
    }
    return count;
  default:
    return -1;
  }
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
  case SYS_write:
    c->GPRx = sys_write(a[1], (void *)a[2], a[3]);
    break;
  default:
    panic("Unhandled syscall ID = %d", a[0]);
  }
}

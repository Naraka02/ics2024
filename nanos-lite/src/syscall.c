#include "syscall.h"
#include "am.h"
#include <common.h>
#include <stdint.h>

#define ssize_t int
#define off_t int

int fs_open(const char *pathname, int flags, int mode);
size_t fs_read(int fd, void *buf, size_t len);
size_t fs_write(int fd, const void *buf, size_t len);
size_t fs_lseek(int fd, size_t offset, int whence);

int fs_close(int fd);
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
    return fs_write(fd, buf, count);
  }
}

static inline int sys_brk(int *addr) {
  extern char end;
  static char *brk = &end;
  char *old_brk = brk;
  if (addr == 0) {
    return (int)brk;
  }
  if (addr > (int *)brk) {
    brk = (char *)addr;
  }
  return (int)old_brk;
}

static inline int sys_open(const char *pathname, int flags, int mode) {
  return fs_open(pathname, flags, mode);
}

static inline size_t sys_read(int fd, void *buf, size_t count) {
  return fs_read(fd, buf, count);
}

static inline int sys_close(int fd) { return fs_close(fd); }

static inline off_t sys_lseek(int fd, off_t offset, int whence) {
  return fs_lseek(fd, offset, whence);
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
  case SYS_brk:
    c->GPRx = sys_brk((int *)a[1]);
    break;
  case SYS_open:
    c->GPRx = sys_open((const char *)a[1], a[2], a[3]);
    break;
  case SYS_read:
    c->GPRx = sys_read(a[1], (void *)a[2], a[3]);
    break;
  case SYS_close:
    c->GPRx = sys_close(a[1]);
    break;
  case SYS_lseek:
    c->GPRx = sys_lseek(a[1], a[2], a[3]);
    break;
  default:
    panic("Unhandled syscall ID = %d", a[0]);
  }
}

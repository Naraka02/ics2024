#include "syscall.h"
#include "am.h"
#include <common.h>
#include <proc.h>
#include <stdint.h>

#define ssize_t int
#define off_t int
struct timeval {
  long tv_sec;
  long tv_usec;
};

int fs_open(const char *pathname, int flags, int mode);
size_t fs_read(int fd, void *buf, size_t len);
size_t fs_write(int fd, const void *buf, size_t len);
size_t fs_lseek(int fd, size_t offset, int whence);
void naive_uload(PCB *pcb, const char *filename);
void context_uload(PCB *pcb, const char *filename, char *const argv[],
                   char *const envp[]);
void switch_boot_pcb();
int mm_brk(uintptr_t brk);

int fs_close(int fd);
static inline void sys_yield(Context *c) {
  yield();
  c->GPRx = 0;
}

static inline ssize_t sys_write(int fd, const void *buf, size_t count) {
  return fs_write(fd, buf, count);
}

static inline int sys_brk(int *addr) { return mm_brk((uintptr_t)addr); }

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

static inline int sys_gettimeofday(void *tv, void *tz) {
  assert(tz == NULL);
  uint64_t us = io_read(AM_TIMER_UPTIME).us;
  ((struct timeval *)tv)->tv_sec = us / 1000000;
  ((struct timeval *)tv)->tv_usec = us % 1000000;
  return 0;
}

static inline int sys_execve(const char *filename, char *const argv[],
                             char *const envp[]) {
  int fd = fs_open(filename, 0, 0);
  if (fd < 0) {
    return -2;
  }
  fs_close(fd);
  context_uload(current, filename, argv, NULL);
  switch_boot_pcb();
  yield();
  return 0;
}

static inline int sys_exit(int status) {
  halt(0);
  // sys_execve("/bin/nterm", NULL, NULL);
  return 0;
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
    c->GPRx = sys_exit(a[1]);
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
  case SYS_gettimeofday:
    c->GPRx = sys_gettimeofday((void *)a[1], (void *)a[2]);
    break;
  case SYS_execve:
    c->GPRx = sys_execve((const char *)a[1], (char *const *)a[2],
                         (char *const *)a[3]);
    break;
  default:
    panic("Unhandled syscall ID = %d", a[0]);
  }
}

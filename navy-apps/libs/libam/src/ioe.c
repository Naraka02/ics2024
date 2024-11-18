#include <am.h>

int open(const char *pathname, int flags, ...);

bool ioe_init() { return true; }

void ioe_read(int reg, void *buf) {
  int fd = open("/dev/am", 0, 0);
  lseek(fd, reg, SEEK_SET);
  read(fd, buf, 0);
  close(fd);
}
void ioe_write(int reg, void *buf) {
  int fd = open("/dev/am", 0, 0);
  lseek(fd, reg, SEEK_SET);
  write(fd, buf, 0);
  close(fd);
}

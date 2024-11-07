#include <fs.h>

typedef size_t (*ReadFn)(void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn)(const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
} Finfo;

enum { FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB };

static size_t open_offset = 0;

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
    [FD_STDIN] = {"stdin", 0, 0, invalid_read, invalid_write},
    [FD_STDOUT] = {"stdout", 0, 0, invalid_read, invalid_write},
    [FD_STDERR] = {"stderr", 0, 0, invalid_read, invalid_write},
#include "files.h"
};

int fs_open(const char *pathname, int flags, int mode) {
  assert(pathname != NULL);
  for (int i = 0; i < sizeof(file_table) / sizeof(Finfo); i++) {
    Log("file_table[%d].name = %s", i, file_table[i].name);

    if (strcmp(pathname, file_table[i].name) == 0) {
      open_offset = 0;
      return i;
    }
  }
  return -1;
}

size_t fs_read(int fd, void *buf, size_t len) {
  Log("fs_read %d %d %d", fd, open_offset, len);
  Log("open_offset = %d", open_offset);
  assert(open_offset + len <= file_table[fd].size);
  return file_table[fd].read(buf, file_table[fd].disk_offset + open_offset,
                             len);
}

size_t fs_write(int fd, const void *buf, size_t len) {
  assert(open_offset + len <= file_table[fd].size);
  return file_table[fd].write(buf, file_table[fd].disk_offset + open_offset,
                              len);
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  switch (whence) {
  case SEEK_SET:
    open_offset = offset;
    break;
  case SEEK_CUR:
    open_offset += offset;
    break;
  case SEEK_END:
    open_offset = file_table[fd].size + offset;
    break;
  default:
    panic("invalid whence");
  }
  assert(open_offset <= file_table[fd].size);
  return open_offset;
}

int fs_close(int fd) { return 0; }

void init_fs() {
  // TODO: initialize the size of /dev/fb
  int FD_SIZE = sizeof(file_table) / sizeof(Finfo);
  for (int i = FD_FB; i < FD_SIZE; i++) {
    file_table[i].read = ramdisk_read;
    file_table[i].write = ramdisk_write;
  }
}

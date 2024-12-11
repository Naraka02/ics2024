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
size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);
size_t am_read(void *buf, size_t offset, size_t len);
size_t am_write(const void *buf, size_t offset, size_t len);
size_t sb_write(const void *buf, size_t offset, size_t len);
size_t sbctl_read(void *buf, size_t offset, size_t len);
size_t sbctl_write(const void *buf, size_t offset, size_t len);

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
    [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
    [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
    [FD_FB] = {"/dev/fb", 0, 0, invalid_read, fb_write},
    {"/dev/events", 0, 0, events_read, invalid_write},
    {"/dev/am", 0, 0, am_read, am_write},
    {"/dev/sb", 0, 0, invalid_read, sb_write},
    {"/dev/sbctl", 0, 0, sbctl_read, sbctl_write},
    {"/proc/dispinfo", 0, 0, dispinfo_read, invalid_write},
#include "files.h"
};

int fs_open(const char *pathname, int flags, int mode) {
  assert(pathname != NULL);
  for (int i = 0; i < sizeof(file_table) / sizeof(Finfo); i++) {
    if (strcmp(pathname, file_table[i].name) == 0) {
      open_offset = 0;
      return i;
    }
  }
  Log("no such file: %s", pathname);
  return -1;
}

size_t fs_read(int fd, void *buf, size_t len) {
  len = open_offset + len <= file_table[fd].size || file_table[fd].size == 0
            ? len
            : file_table[fd].size - open_offset;
  open_offset += len;
  return file_table[fd].read(
      buf, file_table[fd].disk_offset + open_offset - len, len);
}

size_t fs_write(int fd, const void *buf, size_t len) {
  len = open_offset + len <= file_table[fd].size || file_table[fd].size == 0
            ? len
            : file_table[fd].size - open_offset;
  open_offset += len;
  return file_table[fd].write(
      buf, file_table[fd].disk_offset + open_offset - len, len);
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
  AM_GPU_CONFIG_T cfg = io_read(AM_GPU_CONFIG);
  file_table[FD_FB].size = cfg.width * cfg.height * 4;
  int table_size = sizeof(file_table) / sizeof(Finfo);
  for (int i = 0; i < table_size; i++) {
    if (file_table[i].read == NULL) {
      file_table[i].read = ramdisk_read;
    }
    if (file_table[i].write == NULL) {
      file_table[i].write = ramdisk_write;
    }
  }
}

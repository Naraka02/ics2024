#include <sdl-file.h>
#include <stdlib.h>
#include <string.h>

static int64_t file_size(SDL_RWops *f) {
  if (f->fp == NULL)
    return -1;

  long curr = ftell(f->fp);
  if (curr < 0)
    return -1;

  if (fseek(f->fp, 0, SEEK_END) != 0)
    return -1;

  long endpos = ftell(f->fp);
  if (endpos < 0) {
    fseek(f->fp, curr, SEEK_SET);
    return -1;
  }

  fseek(f->fp, curr, SEEK_SET);
  return (int64_t)endpos;
}

static int64_t file_seek(SDL_RWops *f, int64_t offset, int whence) {
  if (f->fp == NULL)
    return -1;

  if (fseek(f->fp, (long)offset, whence) != 0) {
    return -1;
  }

  long pos = ftell(f->fp);
  return (pos < 0) ? -1 : (int64_t)pos;
}

static size_t file_read(SDL_RWops *f, void *buf, size_t size, size_t nmemb) {
  if (f->fp == NULL)
    return 0;
  return fread(buf, size, nmemb, f->fp);
}

static size_t file_write(SDL_RWops *f, const void *buf, size_t size,
                         size_t nmemb) {
  if (f->fp == NULL)
    return 0;
  return fwrite(buf, size, nmemb, f->fp);
}

static int file_close(SDL_RWops *f) {
  int ret = 0;
  if (f->fp) {
    ret = fclose(f->fp);
  }
  free(f);
  return ret;
}

SDL_RWops *SDL_RWFromFile(const char *filename, const char *mode) {
  FILE *fp = fopen(filename, mode);
  if (!fp) {
    return NULL;
  }

  SDL_RWops *rw = (SDL_RWops *)calloc(1, sizeof(SDL_RWops));
  if (!rw) {
    fclose(fp);
    return NULL;
  }

  rw->size = file_size;
  rw->seek = file_seek;
  rw->read = file_read;
  rw->write = file_write;
  rw->close = file_close;

  rw->type = RW_TYPE_FILE;
  rw->fp = fp;

  return rw;
}

static int64_t mem_size(SDL_RWops *f) { return f->mem.size; }

static int64_t mem_seek(SDL_RWops *f, int64_t offset, int whence) {
  int64_t newpos = 0;
  switch (whence) {
  case RW_SEEK_SET:
    newpos = offset;
    break;
  case RW_SEEK_CUR:
    newpos = f->mem.offset + offset;
    break;
  case RW_SEEK_END:
    newpos = f->mem.size + offset;
    break;
  default:
    return -1;
  }

  if (newpos < 0) {
    newpos = 0;
  } else if (newpos > f->mem.size) {
    newpos = f->mem.size;
  }

  f->mem.offset = newpos;
  return newpos;
}

static size_t mem_read(SDL_RWops *f, void *buf, size_t size, size_t nmemb) {
  size_t total = size * nmemb;
  if (total == 0)
    return 0;

  size_t remaining = (size_t)(f->mem.size - f->mem.offset);
  if (remaining < total) {
    total = remaining;
  }

  memcpy(buf, (uint8_t *)f->mem.base + f->mem.offset, total);
  f->mem.offset += total;

  return (total / size);
}

static size_t mem_write(SDL_RWops *f, const void *buf, size_t size,
                        size_t nmemb) {
  size_t total = size * nmemb;
  if (total == 0)
    return 0;

  size_t remaining = (size_t)(f->mem.size - f->mem.offset);
  if (remaining < total) {
    total = remaining;
  }

  memcpy((uint8_t *)f->mem.base + f->mem.offset, buf, total);
  f->mem.offset += total;

  return total / size;
}

static int mem_close(SDL_RWops *f) {
  free(f);
  return 0;
}

SDL_RWops *SDL_RWFromMem(void *mem, int size) {
  if (!mem || size < 0) {
    return NULL;
  }

  SDL_RWops *rw = (SDL_RWops *)calloc(1, sizeof(SDL_RWops));
  if (!rw)
    return NULL;

  rw->size = mem_size;
  rw->seek = mem_seek;
  rw->read = mem_read;
  rw->write = mem_write;
  rw->close = mem_close;

  rw->type = RW_TYPE_MEM;
  rw->mem.base = mem;
  rw->mem.size = size;
  rw->mem.offset = 0;

  return rw;
}

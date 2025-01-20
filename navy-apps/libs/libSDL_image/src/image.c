#define SDL_malloc malloc
#define SDL_free free
#define SDL_realloc realloc

#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"

SDL_Surface *IMG_Load_RW(SDL_RWops *src, int freesrc) {
  assert(src->type == RW_TYPE_MEM);
  assert(freesrc == 0);

  SDL_Surface *ret = STBIMG_LoadFromMemory(src->mem.base, src->mem.size);
  return ret;
}

SDL_Surface *IMG_Load(const char *filename) {
  FILE *fp = fopen(filename, "rb");
  fseek(fp, 0, SEEK_END);
  int size = ftell(fp);

  unsigned char *buf = (unsigned char *)malloc(size);
  fseek(fp, 0, SEEK_SET);
  fread(buf, 1, size, fp);
  SDL_Surface *ret = STBIMG_LoadFromMemory(buf, size);

  free(buf);
  fclose(fp);
  return ret;
}

int IMG_isPNG(SDL_RWops *src) {
  unsigned char magic[8];
  static const unsigned char pngsig[8] = {0x89, 0x50, 0x4E, 0x47,
                                          0x0D, 0x0A, 0x1A, 0x0A};

  Sint64 start_pos = SDL_RWtell(src);
  if (start_pos < 0) {
    return 0;
  }

  size_t n = SDL_RWread(src, magic, 1, 8);
  if (n < 8) {
    SDL_RWseek(src, start_pos, RW_SEEK_SET);
    return 0;
  }

  int is_png = (memcmp(magic, pngsig, 8) == 0);

  SDL_RWseek(src, start_pos, RW_SEEK_SET);

  return is_png ? 1 : 0;
}

SDL_Surface *IMG_LoadJPG_RW(SDL_RWops *src) { return IMG_Load_RW(src, 0); }

char *IMG_GetError() { return "Navy does not support IMG_GetError()"; }

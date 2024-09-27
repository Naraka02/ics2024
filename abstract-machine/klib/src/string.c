#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  const char *sc;
  for (sc = s; *sc != '\0'; ++sc);
  return (sc - s);
}

char *strcpy(char *dst, const char *src) {
  char *s = dst;
  for (s = dst; (*s++ = *src++) != '\0'; );
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  char *s;
  for (s = dst; 0 < n && *src != '\0'; --n) 
    *s++ = *src++;
  for (; 0 < n; --n)
    *s++ = '\0';
  return dst;
}

char *strcat(char *dst, const char *src) {
  char *s;
  for (s = dst; *s != '\0'; ++s);
  for (; (*s = *src) != '\0'; ++s, ++src);
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  for (; *s1 == *s2; ++s1, ++s2)
    if (*s1 == '\0') return 0;
  return (*(unsigned char *)s1 < *(unsigned char *)s2 ? -1 : 1);
}

int strncmp(const char *s1, const char *s2, size_t n) {
  for (; 0 < n; ++s1, ++s2, --n)
    if (*s1 != *s2)
      return (*(unsigned char *)s1 < *(unsigned char *)s2 ? -1 : 1);
    else if (*s1 == '\0')
      return 0;
  return 0;
}

void *memset(void *s, int c, size_t n) {
  const unsigned char uc = c;
  unsigned char *su = (unsigned char *)s;
  for (; 0 < n; ++su, --n)
    *su = uc;
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  char *sc1 = (char *)dst;
  const char *sc2 = (const char *)src;
  if (sc2 < sc1 && sc1 < sc2 + n)
    for (sc1 +=n, sc2 +=n; 0 < n; --n)
      *--sc1 = *--sc2;
  else
    for (; 0 < n; --n)
      *sc1++ = *sc2++;
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  char *su1 = (char *)out;
  const char *su2 = (const char *)in;
  for (; 0 < n; ++su1, ++su2, --n)
    *su1 = *su2;
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const unsigned char *su1 = (const unsigned char *)s1;
  const unsigned char *su2 = (const unsigned char *)s2;
  for (; 0 < n; ++su1, ++su2, --n)
    if (*su1 != *su2)
      return (*su1 < *su2 ? -1 : 1);
  return 0;
}

#endif

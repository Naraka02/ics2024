#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define BUFSIZE 4096

static inline int print_int(int num, char *buf) {
  int num_len = 0;
  int is_neg = num < 0;
  if (is_neg) {
    buf[0] = '-';
    buf++;
  }
  int tmp = num;
  do {
    tmp /= 10;
    num_len++;
  } while (tmp);
  int digit;
  for (int i = num_len - 1; i >= 0; --i) {
    digit = num % 10 < 0 ? -(num % 10) : num % 10;
    buf[i] = digit + '0';
    num /= 10;
  }
  return num_len + is_neg;
}

static inline int print_hex(unsigned int num, char *buf) {
  int num_len = 0;
  int tmp = num;
  do {
    tmp /= 16;
    num_len++;
  } while (tmp);
  int digit;
  for (int i = num_len - 1; i >= 0; --i) {
    digit = num % 10;
    buf[i] = digit < 10 ? digit + '0' : digit - 10 + 'a';
    num /= 10;
  }
  return num_len + num < 0; 
}

int _Printf(char *buf, const char *fmt, va_list ap) {
  int len = 0;
  for (const char* p = fmt; *p != '\0'; p++) {
    switch (*p) {
      case '%':
        switch (*++p) {
          case 'd':
            len += print_int(va_arg(ap, int), buf + len);
            break;
          case 's':
            const char* str = va_arg(ap, const char*);
            while (*str != '\0') {
              buf[len++] = *str++;
            }
            break;
          case 'c':
            buf[len++] = va_arg(ap, int);
            break;
          case 'x':
            print_hex(va_arg(ap, unsigned int), buf + len);
            break;
          case 'p':
            buf[len++] = '0';
            buf[len++] = 'x';
            print_hex(va_arg(ap, unsigned int), buf + len);
            break;
        }
        break;
      default:
        buf[len++] = *p;
        break;
    }
  }
  buf[len++] = '\0';
  return 0;
}

int printf(const char *fmt, ...) {
  int ans;
  va_list ap;
  char buf[BUFSIZE];

  va_start(ap, fmt);
  ans = _Printf(buf, fmt, ap);
  for (int i = 0; buf[i] != '\0'; ++i) {
    putch(buf[i]);
  }
  va_end(ap);

  return ans;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  int ans;
  va_list ap;
  char buf[BUFSIZE];

  va_start(ap, fmt);
  ans = _Printf(buf, fmt, ap);
  memcpy(out, buf, strlen(buf) + 1);
  va_end(ap);

  return ans;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  int ans;
  va_list ap;
  char buf[1024];

  va_start(ap, fmt);
  ans = _Printf(buf, fmt, ap);
  memcpy(out, buf, n);
  va_end(ap);

  return ans;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif

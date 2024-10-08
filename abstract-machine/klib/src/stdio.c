#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  int ans, len;
  va_list ap;
  char buf[1024];

  ans = 0;
  len = 0;
  va_start(ap, fmt);
  for (const char* p = fmt; *p != '\0'; ++p) {
    switch (*p) {
      case '%':
        switch (*++p) {
          case 'd':
            int num = va_arg(ap, int); 
            int num_len = 0;
            if (num < 0) {
              buf[len++] = '-';
              num = -num;
            }
            int num_copy = num;
            do {
              num_copy /= 10;
              num_len++;
            } while (num_copy);
            buf[len + num_len] = '\0';
            do {
              buf[len + --num_len] = num % 10 + '0';
              num /= 10;
            } while (num);
            len += num_len;
            break;
          case 's':
            const char* str = va_arg(ap, const char*);
            while (*str) {
              buf[len++] = *str++;
            }
            break;
        }
        break;
      default:
        buf[len++] = *p;
        break;
    }
  }
  memcpy(out, buf, len);
  va_end(ap);

  return ans;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif

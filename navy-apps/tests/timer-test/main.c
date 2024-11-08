#include <assert.h>
#include <stdio.h>

int gettimeofday(struct timeval *tv, void *tz);

int main() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  int sec = tv.tv_sec;
  while (1) {
    gettimeofday(&tv, NULL);
    if (tv.tv_sec > sec) {
      printf("1 second passed\n");
      sec = tv.tv_sec;
    }
  }
}

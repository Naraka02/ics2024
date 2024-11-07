#include <assert.h>
#include <stdio.h>

int gettimeofday(struct timeval *tv, void *tz);

int main() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  long us = tv.tv_usec;
  printf("us = %ld\n", us);
  while (1) {
    gettimeofday(&tv, NULL);
    printf("tv.tv_usec = %d\n", tv.tv_usec);
    if (tv.tv_usec - us >= 500000) {
      printf("0.5 second passed\n");
      us = tv.tv_usec;
    }
  }
}

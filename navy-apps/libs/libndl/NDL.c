#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;
static struct timeval NDL_InitTime;

int gettimeofday(struct timeval *tv, void *tz);
int open(const char *pathname, int flags, ...);

uint32_t NDL_GetTicks() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec - NDL_InitTime.tv_sec) * 1000 +
         (tv.tv_usec - NDL_InitTime.tv_usec) / 1000;
}

int NDL_PollEvent(char *buf, int len) {
  int fd = open("/dev/events", 0);
  int ret = read(fd, buf, len);
  close(fd);
  return ret;
}

void NDL_OpenCanvas(int *w, int *h) {
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w;
    screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0)
        continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0)
        break;
    }
    close(fbctl);
  }
  int fd = open("/proc/dispinfo", 0);
  char buf[64];
  read(fd, buf, 64);
  sscanf(buf, "WIDTH: %d\nHEIGHT: %d\n", &screen_w, &screen_h);
  if (*w == 0 && *h == 0) {
    *w = screen_w;
    *h = screen_h;
  }
  if (*w > screen_w)
    *w = screen_w;
  if (*h > screen_h)
    *h = screen_h;
  close(fd);
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  int fd = open("/dev/fb", 0);
  int nx = (screen_w - w) / 2, ny = (screen_h - h) / 2;
  for (int j = 0; j < h; j++) {
    lseek(fd, ((ny + j) * screen_w + nx) * sizeof(uint32_t), SEEK_SET);
    write(fd, pixels + j * w, w * sizeof(uint32_t));
  }
  close(fd);
}

void NDL_OpenAudio(int freq, int channels, int samples) {
  int fd = open("/dev/sbctl", 0);
  int args[3] = {freq, channels, samples};
  write(fd, args, sizeof(args));
  close(fd);
}

void NDL_CloseAudio() {}

int NDL_PlayAudio(void *buf, int len) {
  int fd = open("/dev/sb", 0);
  int ret = write(fd, buf, len);
  close(fd);
  return ret;
}

int NDL_QueryAudio() {
  int fd = open("/dev/sbctl", 0);
  int ret;
  read(fd, &ret, sizeof(ret));
  close(fd);
  printf("NDL_QueryAudio: %d\n", ret);
  return ret;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  gettimeofday(&NDL_InitTime, NULL);
  return 0;
}

void NDL_Quit() {}

#include <common.h>
#include <stdio.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
#define MULTIPROGRAM_YIELD() yield()
#else
#define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) [AM_KEY_##key] = #key,

static const char *keyname[256]
    __attribute__((used)) = {[AM_KEY_NONE] = "NONE", AM_KEYS(NAME)};

size_t serial_write(const void *buf, size_t offset, size_t len) {
  for (size_t i = 0; i < len; i++) {
    putch(((char *)buf)[i]);
  }
  return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  if (ev.keycode == AM_KEY_NONE) {
    return 0;
  }
  return snprintf(buf, len, "%s %s\n", ev.keydown ? "kd" : "ku",
                  keyname[ev.keycode]);
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  AM_GPU_CONFIG_T cfg = io_read(AM_GPU_CONFIG);
  snprintf(buf, len, "WIDTH: %d\nHEIGHT: %d\n", cfg.width, cfg.height);
  return len;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  AM_GPU_CONFIG_T cfg = io_read(AM_GPU_CONFIG);
  int x = offset / 4 % cfg.width, y = offset / 4 / cfg.width;
  io_write(AM_GPU_FBDRAW, x, y, (void *)buf, len / 4, 1, true);
  return len;
}

size_t am_read(void *buf, size_t offset, size_t len) {
  ioe_read(offset, buf);
  return len;
}

size_t am_write(const void *buf, size_t offset, size_t len) {
  ioe_write(offset, (void *)buf);
  return len;
}

size_t sb_write(const void *buf, size_t offset, size_t len) {
  int sbuf_size = io_read(AM_AUDIO_CONFIG).bufsize;
  while (len > sbuf_size - io_read(AM_AUDIO_STATUS).count)
    ;
  io_write(AM_AUDIO_PLAY, (Area){(void *)buf, (void *)buf + len});
  return len;
}

size_t sbctl_read(void *buf, size_t offset, size_t len) {
  AM_AUDIO_STATUS_T stat = io_read(AM_AUDIO_STATUS);
  AM_AUDIO_CONFIG_T cfg = io_read(AM_AUDIO_CONFIG);
  snprintf(buf, len, "%d", cfg.bufsize - stat.count);
  printf("sbctl_read: %d\n", *(int *)buf);
  return len;
}

size_t sbctl_write(const void *buf, size_t offset, size_t len) {
  int freq = *(int *)buf;
  int channels = *(int *)(buf + 4);
  int samples = *(int *)(buf + 8);
  io_write(AM_AUDIO_CTRL, freq, channels, samples);
  return len;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}

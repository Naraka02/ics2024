#include <common.h>

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
  int x = offset % cfg.width, y = offset / cfg.width;
  io_write(AM_GPU_FBDRAW, x, y, (void *)buf, len / 4, 1, true);
  return len;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}

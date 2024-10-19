#include <am.h>
#include <nemu.h>
#include <stdio.h>

#define AUDIO_FREQ_ADDR      (AUDIO_ADDR + 0x00)
#define AUDIO_CHANNELS_ADDR  (AUDIO_ADDR + 0x04)
#define AUDIO_SAMPLES_ADDR   (AUDIO_ADDR + 0x08)
#define AUDIO_SBUF_SIZE_ADDR (AUDIO_ADDR + 0x0c)
#define AUDIO_INIT_ADDR      (AUDIO_ADDR + 0x10)
#define AUDIO_COUNT_ADDR     (AUDIO_ADDR + 0x14)

void __am_audio_init() {
  outl(AUDIO_INIT_ADDR, 1);
}

void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  cfg->present = true;
  cfg->bufsize = inl(AUDIO_SBUF_SIZE_ADDR);
}

void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
  outl(AUDIO_FREQ_ADDR, ctrl->freq);
  printf("samples = %d\n", ctrl->samples);
  outl(AUDIO_CHANNELS_ADDR, ctrl->channels);
  outl(AUDIO_SAMPLES_ADDR, ctrl->samples);
  __am_audio_init();
}

void __am_audio_status(AM_AUDIO_STATUS_T *stat) {
  stat->count = inl(AUDIO_COUNT_ADDR);
}

void __am_audio_play(AM_AUDIO_PLAY_T *ctl) {
  Area buf = ctl->buf;
  int len = buf.end - buf.start;
  int count = io_read(AM_AUDIO_STATUS).count;
  int sbuf_size = io_read(AM_AUDIO_CONFIG).bufsize;
  while (len > sbuf_size - count) {
    count = io_read(AM_AUDIO_STATUS).count;
  }
  outl(AUDIO_COUNT_ADDR, count + len);
  uintptr_t addr = AUDIO_SBUF_ADDR + count;
  for (int i = 0; i < len; i++) {
    outb(addr + i, *(uint8_t *)(buf.start + i));
  }
}

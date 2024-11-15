#include <am.h>
#include <sys/time.h>

static void __am_uart_config(AM_UART_CONFIG_T *cfg) { cfg->present = false; }
static void __am_uart_tx(AM_UART_TX_T *tx) { tx->data = 0; }
static void __am_uart_rx(AM_UART_RX_T *rx) { rx->data = 0; }
static void __am_timer_config(AM_TIMER_CONFIG_T *cfg) {
  cfg->present = true;
  cfg->has_rtc = true;
}
static void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->year = 0;
  rtc->month = 0;
  rtc->day = 0;
  rtc->hour = 0;
  rtc->minute = 0;
  rtc->second = 0;
}
static void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  uptime->us = tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
static void __am_input_config(AM_INPUT_CONFIG_T *cfg) { cfg->present = false; }
static void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  kbd->keydown = false;
  kbd->keycode = AM_KEY_NONE;
}
static void __am_gpu_config(AM_GPU_CONFIG_T *cfg) { cfg->present = false; }
static void __am_gpu_status(AM_GPU_STATUS_T *stat) { stat->ready = false; }
static void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *draw) {}
static void __am_audio_config(AM_AUDIO_CONFIG_T *cfg) {
  cfg->present = false;
  cfg->bufsize = 0;
}
static void __am_audio_ctrl(AM_AUDIO_CTRL_T *ctrl) {
  ctrl->freq = 0;
  ctrl->channels = 0;
  ctrl->samples = 0;
}
static void __am_audio_status(AM_AUDIO_STATUS_T *stat) { stat->count = 0; }
static void __am_audio_play(AM_AUDIO_PLAY_T *play) {}
static void __am_disk_config(AM_DISK_CONFIG_T *cfg) {
  cfg->present = false;
  cfg->blksz = 0;
  cfg->blkcnt = 0;
}
static void __am_disk_status(AM_DISK_STATUS_T *stat) { stat->ready = false; }
static void __am_disk_blkio(AM_DISK_BLKIO_T *io) {}
static void __am_net_config(AM_NET_CONFIG_T *cfg) { cfg->present = false; }
static void __am_net_status(AM_NET_STATUS_T *stat) {
  stat->rx_len = 0;
  stat->tx_len = 0;
}
static void __am_net_tx(AM_NET_TX_T *tx) {}
static void __am_net_rx(AM_NET_RX_T *rx) {}

typedef void (*handler_t)(void *buf);
static void *lut[128] = {
    [AM_UART_CONFIG] = __am_uart_config,
    [AM_UART_TX] = __am_uart_tx,
    [AM_UART_RX] = __am_uart_rx,
    [AM_TIMER_CONFIG] = __am_timer_config,
    [AM_TIMER_RTC] = __am_timer_rtc,
    [AM_TIMER_UPTIME] = __am_timer_uptime,
    [AM_INPUT_CONFIG] = __am_input_config,
    [AM_INPUT_KEYBRD] = __am_input_keybrd,
    [AM_GPU_CONFIG] = __am_gpu_config,
    [AM_GPU_STATUS] = __am_gpu_status,
    [AM_GPU_FBDRAW] = __am_gpu_fbdraw,
    [AM_AUDIO_CONFIG] = __am_audio_config,
    [AM_AUDIO_CTRL] = __am_audio_ctrl,
    [AM_AUDIO_STATUS] = __am_audio_status,
    [AM_AUDIO_PLAY] = __am_audio_play,
    [AM_DISK_CONFIG] = __am_disk_config,
    [AM_DISK_STATUS] = __am_disk_status,
    [AM_DISK_BLKIO] = __am_disk_blkio,
    [AM_NET_CONFIG] = __am_net_config,
    [AM_NET_STATUS] = __am_net_status,
    [AM_NET_TX] = __am_net_tx,
    [AM_NET_RX] = __am_net_rx,
};

bool ioe_init() {
  return true;
}

void ioe_read(int reg, void *buf) { ((handler_t)lut[reg])(buf); }
void ioe_write(int reg, void *buf) { ((handler_t)lut[reg])(buf); }

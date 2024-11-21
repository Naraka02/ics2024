#include <NDL.h>
#include <SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static uint32_t timer = 0;
static uint32_t interval = 0;
static uint8_t *stream = NULL;
static int samples = 0;
static void (*SDL_Callback)(void *, uint8_t *, int) = NULL;
static int is_init = 0;
static int is_recall = 0;

void CallBackHelper() {
  if (SDL_GetTicks() - timer < interval || !is_init || is_recall)
    return;
  is_recall = 1;
  timer = SDL_GetTicks();
  int free = NDL_QueryAudio();
  SDL_Callback(NULL, stream, free > samples ? samples : free);
  NDL_PlayAudio(stream, samples);
  is_recall = 0;
}

int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained) {
  interval = 1000 / desired->freq;
  stream = (uint8_t *)malloc(desired->samples);
  samples = desired->samples;
  SDL_Callback = desired->callback;
  NDL_OpenAudio(desired->freq, desired->channels, desired->samples);
  is_init = 1;
  return 0;
}

void SDL_CloseAudio() {}

void SDL_PauseAudio(int pause_on) {
  if (pause_on > 0)
    return;
  CallBackHelper();
}

void SDL_MixAudio(uint8_t *dst, uint8_t *src, uint32_t len, int volume) {
  for (int i = 0; i < len; i++) {
    dst[i] = (dst[i] * volume + src[i] * (128 - volume)) / 128;
  }
}

SDL_AudioSpec *SDL_LoadWAV(const char *file, SDL_AudioSpec *spec,
                           uint8_t **audio_buf, uint32_t *audio_len) {
  FILE *fp = fopen(file, "r");

  uint16_t NumChannels, BitsPerSample;
  uint32_t SampleRate;
  fseek(fp, 22, SEEK_SET);
  fread(&NumChannels, 2, 1, fp);
  fseek(fp, 34, SEEK_SET);
  fread(&BitsPerSample, 2, 1, fp);
  fseek(fp, 24, SEEK_SET);
  fread(&SampleRate, 4, 1, fp);

  spec->freq = SampleRate;
  spec->channels = NumChannels;
  spec->format = BitsPerSample == 8 ? AUDIO_U8 : AUDIO_S16SYS;

  fseek(fp, 44, SEEK_SET);
  printf("audio_len: %d\n", *audio_len);
  fread(audio_buf, *audio_len, 1, fp);
  fclose(fp);

  return spec;
}

void SDL_FreeWAV(uint8_t *audio_buf) { free(audio_buf); }

void SDL_LockAudio() {}

void SDL_UnlockAudio() {}

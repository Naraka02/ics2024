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

void CallBackHelper() {
  if (SDL_GetTicks() - timer < interval)
    return;
  timer = SDL_GetTicks();
  int free = NDL_QueryAudio();
  SDL_Callback(NULL, stream, free > samples ? samples : free);
  NDL_PlayAudio(stream, samples);
}

int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained) {
  interval = 1000 / desired->freq;
  stream = (uint8_t *)malloc(desired->samples);
  samples = desired->samples;
  SDL_Callback = desired->callback;
  NDL_OpenAudio(desired->freq, desired->channels, desired->samples);

  CallBackHelper();
  return 0;
}

void SDL_CloseAudio() {}

void SDL_PauseAudio(int pause_on) {
  if (pause_on > 0)
    return;
  CallBackHelper();
}

void SDL_MixAudio(uint8_t *dst, uint8_t *src, uint32_t len, int volume) {}

SDL_AudioSpec *SDL_LoadWAV(const char *file, SDL_AudioSpec *spec,
                           uint8_t **audio_buf, uint32_t *audio_len) {
  return NULL;
}

void SDL_FreeWAV(uint8_t *audio_buf) {}

void SDL_LockAudio() {}

void SDL_UnlockAudio() {}

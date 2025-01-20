#include <NDL.h>
#include <sdl-timer.h>
#include <stdio.h>

#define MAX_TIMERS 16

typedef struct {
  int used;
  SDL_TimerID id;
  uint32_t interval;
  SDL_NewTimerCallback callback;
  void *param;
  uint32_t last_trigger;
} TimerEntry;

static TimerEntry timers[MAX_TIMERS];

void SDL_TimerUpdate() {
  uint32_t now = NDL_GetTicks();
  for (int i = 0; i < MAX_TIMERS; i++) {
    if (timers[i].used) {
      if ((now - timers[i].last_trigger) >= timers[i].interval) {
        timers[i].last_trigger = now;
        timers[i].callback(timers[i].interval, timers[i].param);
      }
    }
  }
}

SDL_TimerID SDL_AddTimer(uint32_t interval, SDL_NewTimerCallback callback,
                         void *param) {
  for (int i = 0; i < MAX_TIMERS; i++) {
    if (!timers[i].used) {
      timers[i].used = 1;
      timers[i].id = i + 1;
      timers[i].interval = interval;
      timers[i].callback = callback;
      timers[i].param = param;
      timers[i].last_trigger = NDL_GetTicks();
      return timers[i].id;
    }
  }

  return 0;
}

int SDL_RemoveTimer(SDL_TimerID id) {
  if (id <= 0)
    return 0;
  int idx = id - 1;
  if (idx >= MAX_TIMERS)
    return 0;

  timers[idx].used = 0;
  return 1;
}

uint32_t SDL_GetTicks() { return NDL_GetTicks(); }

void SDL_Delay(uint32_t ms) {
  uint32_t start = NDL_GetTicks();
  while ((NDL_GetTicks() - start) < ms)
    ;
}

#include <NDL.h>
#include <SDL.h>
#include <string.h>

#define keyname(k) #k,

static const char *keyname[] = {"NONE", _KEYS(keyname)};
#define NUM_KEYS (sizeof(keyname) / sizeof(char))
static uint8_t keystate[NUM_KEYS] = {0};

#define EVENT_QUEUE_LEN 64
static SDL_Event event_queue[EVENT_QUEUE_LEN] = {};
static int event_f = 0, event_r = 0;

void CallBackHelper();

int SDL_PushEvent(SDL_Event *ev) {
  if (event_r == (event_f - 1 + EVENT_QUEUE_LEN) % EVENT_QUEUE_LEN) {
    return 0;
  }
  event_queue[event_r] = *ev;
  event_r = (event_r + 1) % EVENT_QUEUE_LEN;
  return 1;
}

int SDL_PollEvent(SDL_Event *ev) {
  CallBackHelper();

  char buf[64];
  if (NDL_PollEvent(buf, 64) == 0) {
    return 0;
  }
  char type[4], name[16];
  sscanf(buf, "%s %s", type, name);
  ev->type = strcmp(type, "ku") == 0 ? SDL_KEYUP : SDL_KEYDOWN;
  for (int i = 0; i < NUM_KEYS; i++) {
    if (strcmp(name, keyname[i]) == 0) {
      ev->key.type = ev->type;
      ev->key.keysym.sym = i;
      keystate[i] = ev->type == SDL_KEYUP ? 0 : 1;
      return i;
    }
  }
  return 0;
}

int SDL_WaitEvent(SDL_Event *ev) {
  while (1) {
    if (SDL_PollEvent(ev)) {
      return 1;
    }
  }
  return 0;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  int count = 0;
  switch (action) {
  case SDL_GETEVENT:
    if (event_f != event_r) {
      *ev = event_queue[event_f];
      event_f = (event_f + 1) % EVENT_QUEUE_LEN;
      count = 1;
    }
    break;
  }
  return 0;
}

uint8_t *SDL_GetKeyState(int *numkeys) {
  if (numkeys != NULL) {
    *numkeys = NUM_KEYS;
  }
  return keystate;
}

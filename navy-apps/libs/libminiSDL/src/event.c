#include <NDL.h>
#include <SDL.h>
#include <string.h>

#define keyname(k) #k,

static const char *keyname[] = {"NONE", _KEYS(keyname)};
#define NUM_KEYS (sizeof(keyname) / sizeof(char))
static uint8_t keystate[NUM_KEYS] = {0};

void CallBackHelper();

int SDL_PushEvent(SDL_Event *ev) { return 0; }

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
  return 0;
}

uint8_t *SDL_GetKeyState(int *numkeys) {
  if (numkeys != NULL) {
    *numkeys = NUM_KEYS;
  }
  return keystate;
}

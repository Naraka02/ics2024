#include <NDL.h>
#include <SDL.h>
#include <string.h>

#define keyname(k) #k,
#define NUM_KEYS SDLK_PAGEDOWN

static const char *keyname[] = {"NONE", _KEYS(keyname)};
static uint8_t keystate[NUM_KEYS] = {0};

int SDL_PushEvent(SDL_Event *ev) { return 0; }

int SDL_PollEvent(SDL_Event *ev) {
  char buf[64];
  if (NDL_PollEvent(buf, 64) == 0) {
    return 0;
  }
  char type[4], name[16];
  sscanf(buf, "%s %s", type, name);
  ev->type = strcmp(type, "ku") == 0 ? SDL_KEYUP : SDL_KEYDOWN;
  for (int i = 0; i < sizeof(keyname) / sizeof(char); i++) {
    if (strcmp(name, keyname[i]) == 0) {
      ev->key.type = ev->type;
      ev->key.keysym.sym = i;
      keystate[i] = ev->type == SDL_KEYUP ? 0 : 1;
      return i;
    }
  }
  return 1;
}

int SDL_WaitEvent(SDL_Event *ev) {
  char buf[64];
  while (NDL_PollEvent(buf, 64) != 0) {
    char type[4], name[16];
    sscanf(buf, "%s %s", type, name);
    ev->type = strcmp(type, "ku") == 0 ? SDL_KEYUP : SDL_KEYDOWN;
    for (int i = 0; i < sizeof(keyname) / sizeof(char); i++) {
      if (strcmp(name, keyname[i]) == 0) {
        ev->key.type = ev->type;
        ev->key.keysym.sym = i;
        keystate[i] = ev->type == SDL_KEYUP ? 0 : 1;
        return i;
      }
    }
  }

  return 1;
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

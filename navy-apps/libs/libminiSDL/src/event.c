#include <NDL.h>
#include <SDL.h>
#include <string.h>

#define keyname(k) #k,

static const char *keyname[] = {"NONE", _KEYS(keyname)};

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
      return i;
    }
  }
  return 1;
}

int SDL_WaitEvent(SDL_Event *event) {
  char buf[64];
  while (NDL_PollEvent(buf, 64) != 0) {
    char type[4], name[16];
    sscanf(buf, "%s %s", type, name);
    event->type = strcmp(type, "ku") == 0 ? SDL_KEYUP : SDL_KEYDOWN;
    for (int i = 0; i < sizeof(keyname) / sizeof(char); i++) {
      if (strcmp(name, keyname[i]) == 0) {
        event->key.type = event->type;
        event->key.keysym.sym = i;
        return i;
      }
    }
  }

  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t *SDL_GetKeyState(int *numkeys) { return NULL; }

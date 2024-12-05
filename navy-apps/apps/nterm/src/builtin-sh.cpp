#include <SDL.h>
#include <nterm.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>

char handle_key(SDL_Event *ev);

static void sh_printf(const char *format, ...) {
  static char buf[256] = {};
  va_list ap;
  va_start(ap, format);
  int len = vsnprintf(buf, 256, format, ap);
  va_end(ap);
  term->write(buf, len);
}

static void sh_banner() {
  sh_printf("Built-in Shell in NTerm (NJU Terminal)\n\n");
}

static void sh_prompt() { sh_printf("sh> "); }

static void sh_handle_cmd(const char *cmd) {
  char *p = (char *)cmd;
  while (*p != '\n')
    p++;
  *p = '\0';

  char *argv[16];
  int argc = 0;
  p = (char *)cmd;
  for (char *q = p; *q; q++) {
    if (*q == ' ') {
      *q = '\0';
      argv[argc++] = p;
      p = q + 1;
    }
  }
  argv[argc] = NULL;
  if (argc == 0)
    return;
  execvp(argv[0], argv);
}

void builtin_sh_run() {

  sh_banner();
  sh_prompt();
  setenv("PATH", "/bin/", 0);

  while (1) {
    SDL_Event ev;
    if (SDL_PollEvent(&ev)) {
      if (ev.type == SDL_KEYUP || ev.type == SDL_KEYDOWN) {
        const char *res = term->keypress(handle_key(&ev));
        if (res) {
          sh_handle_cmd(res);
          sh_prompt();
        }
      }
    }
    refresh_terminal();
  }
}

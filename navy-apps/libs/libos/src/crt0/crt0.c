#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
  int argc = args[0];
  printf("args = %p\n", args);
  printf("argc = %d\n", argc);
  for (int i = 0; i < argc; i++) {
    printf("argv[%d] = %s\n", i, (char *)args[i + 1]);
  }
  char **argv = (char **)args;
  char **envp = (char **)args[argc + 1];
  environ = envp;
  asm("call  __libc_init_array");
  exit(main(0, argv, envp));
  assert(0);
}

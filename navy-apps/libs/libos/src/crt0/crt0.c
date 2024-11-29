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
  printf("%s\n", (char *)args + 2);
  char **argv = (char **)(args + 1);
  char **envp = (char **)args[2];
  environ = envp;
  asm("call  __libc_init_array");
  exit(main(0, argv, envp));
  assert(0);
}

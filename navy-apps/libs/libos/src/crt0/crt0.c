#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
  int argc = args[0];
  printf("argc = %d\n", argc);
  char **argv = (char **)args[1];
  printf("argv = %p\n", argv);
  for (int i = 0; i < argc; i++) {
    printf("argv[%d] = %s\n", i, argv[i]);
  }
  char **envp = (char **)(args + argc + 2);
  environ = envp;
  asm("call  __libc_init_array");
  exit(main(0, argv, envp));
  assert(0);
}

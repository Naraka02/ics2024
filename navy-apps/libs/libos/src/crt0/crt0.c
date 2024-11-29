#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
  int argc = args[0];
  printf("args = %p\n", args);
  printf("argc = %d\n", argc);
  printf("%s\n", (char *)args + 4);
  char **argv = malloc(argc * sizeof(char *));
  argv[0] = (char *)args + 4;
  for (int i = 1; i < argc; i++) {
    argv[i] = argv[i - 1] + strlen(argv[i - 1]) + 1;
    printf("%s\n", argv[i]);
  }
  char **envp = (char **)args[2];
  environ = envp;
  asm("call  __libc_init_array");
  exit(main(0, argv, envp));
  assert(0);
}

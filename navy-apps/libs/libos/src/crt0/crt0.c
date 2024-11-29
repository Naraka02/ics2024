#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
  int argc = *(int *)args;
  char **argv = (char **)(args + 1);
  char **envp = (char **)(args + argc + 2);
  environ = envp;
  asm("call  __libc_init_array");
  exit(main(argc, argv, envp));
  assert(0);
}

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

extern void __libc_init_array();

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
  int argc = *(int *)args;
  char **argv = (char **)(args + 1);
  char **envp = (char **)(args + argc + 2);
  environ = envp;
  __libc_init_array();
  exit(main(argc, argv, envp));
  assert(0);
}

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
  char *empty[] = {NULL};
  environ = empty;
  asm("call  __libc_init_array");
  exit(main(0, empty, empty));
  assert(0);
}

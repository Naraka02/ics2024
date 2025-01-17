#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
static PCB *fg_pcb = NULL;
PCB *current = NULL;

void naive_uload(PCB *pcb, const char *filename);
void context_uload(PCB *pcb, const char *filename, char *const argv[],
                   char *const envp[]);

void switch_boot_pcb() { current = &pcb_boot; }

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%p' for the %dth time!",
        (uintptr_t)arg, j);
    j++;
    yield();
  }
}

void context_kload(PCB *pcb, void (*entry)(void *), void *arg) {
  Area kstack = {pcb->stack, pcb->stack + STACK_SIZE};
  pcb->cp = kcontext(kstack, entry, arg);
}

void init_proc() {
  context_uload(&pcb[0], "/bin/hello", NULL, NULL);
  // context_kload(&pcb[0], hello_fun, 0);
  context_uload(&pcb[1], "/bin/nterm", NULL, NULL);
  context_uload(&pcb[2], "/bin/pal", NULL, NULL);
  context_uload(&pcb[3], "/bin/nslider", NULL, NULL);
  fg_pcb = &pcb[2];
  switch_boot_pcb();

  Log("Initializing processes...");

  // naive_uload(NULL, "/bin/nterm");
}

void fg_pcb_switch(int keycode) {
  switch (keycode) {
  case AM_KEY_F1:
    fg_pcb = &pcb[1];
    break;
  case AM_KEY_F2:
    fg_pcb = &pcb[2];
    break;
  case AM_KEY_F3:
    fg_pcb = &pcb[3];
    break;
  default:
    break;
  }
}

Context *schedule(Context *prev) {
  current->cp = prev;
  current = (current == &pcb[0] ? fg_pcb : &pcb[0]);
  return current->cp;
}

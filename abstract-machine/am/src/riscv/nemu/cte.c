#include <am.h>
#include <klib.h>
#include <riscv/riscv.h>

static Context *(*user_handler)(Event, Context *) = NULL;

Context *__am_irq_handle(Context *c) {
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
    case 11:
      ev.event = c->GPR1 == -1 ? EVENT_YIELD : EVENT_SYSCALL;
      break;
    default:
      ev.event = EVENT_ERROR;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }
  c->mepc += 4;

  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context *(*handler)(Event, Context *)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  Context *c = (Context *)(kstack.end - sizeof(Context));
  memset(c, 0, sizeof(Context));
  c->mstatus = 0x1800;
  c->mepc = (uintptr_t)entry;
  for (int i = 12; i < 14; i++) {
    c->gpr[i] = (uintptr_t)arg;
  }
  return c;
}

void yield() {
#ifdef __riscv_e
  asm volatile("li a5, -1; ecall");
#else
  asm volatile("li a7, -1; ecall");
#endif
}

bool ienabled() { return false; }

void iset(bool enable) {}

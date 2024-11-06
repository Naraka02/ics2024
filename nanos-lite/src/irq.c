#include <common.h>

static Context *do_event(Event e, Context *c) {
  printf("Handling event: %d\n", e.event);
  switch (e.event) {
  case EVENT_YIELD:
    Log("Yield event");
    break;
  case EVENT_SYSCALL:
    Log("Syscall event");
    break;
  default:
    panic("Unhandled event ID = %d", e.event);
  }

  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}

#include <memory.h>
#include <proc.h>
#include <stdint.h>

static void *pf = NULL;

void *new_page(size_t nr_page) {
  pf += nr_page * PGSIZE;
  return pf;
}

#ifdef HAS_VME
static void *pg_alloc(int n) {
  int nr_page = (n - 1) / PGSIZE + 1;
  void *ptr = new_page(nr_page) - nr_page * PGSIZE;
  memset(ptr, 0, nr_page * PGSIZE);
  return ptr;
}
#endif

void free_page(void *p) { panic("not implement yet"); }

/* The brk() system call handler. */
int mm_brk(uintptr_t brk) {
  Log("%p %p %x", brk, current->max_brk, heap.start);
  return 0;
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}

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
  printf("hello\n");
  if (current->max_brk >= brk) {
    printf("hello\n");
    return 0;
  }
  if (current->max_brk == 0) {
    current->max_brk = brk % PGSIZE == 0 ? brk : (brk / PGSIZE + 1) * PGSIZE;
    void *page = new_page(1);
    map(&current->as, (void *)current->max_brk - PGSIZE, page, 0b1110);
    printf("hello\n");
    return 0;
  }

  int nr_pages = (int)(brk - current->max_brk - 1) / PGSIZE + 1;
  printf("%d\n", nr_pages);
  for (int i = 0; i < nr_pages; i++) {
    void *page = new_page(1);
    map(&current->as, (void *)current->max_brk + i * PGSIZE, page, 0b1110);
  }
  current->max_brk += nr_pages * PGSIZE;
  printf("hello\n");
  return 0;
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}

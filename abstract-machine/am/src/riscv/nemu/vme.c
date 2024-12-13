#include <am.h>
#include <klib.h>
#include <nemu.h>

#define BITMASK(bits) ((1ull << (bits)) - 1)
#define BITS(x, hi, lo) (((x) >> (lo)) & BITMASK((hi) - (lo) + 1))

static AddrSpace kas = {};
static void *(*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void *) = NULL;
static int vme_enable = 0;

static Area segments[] = { // Kernel memory mappings
    NEMU_PADDR_SPACE};

#define USER_SPACE RANGE(0x40000000, 0x80000000)

static inline void set_satp(void *pdir) {
  uintptr_t mode = 1ul << (__riscv_xlen - 1);
  asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> 12)));
}

static inline uintptr_t get_satp() {
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;
}

bool vme_init(void *(*pgalloc_f)(int), void (*pgfree_f)(void *)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);

  int i;
  for (i = 0; i < LENGTH(segments); i++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      map(&kas, va, va, 0);
    }
  }

  set_satp(kas.ptr);
  vme_enable = 1;

  return true;
}

void protect(AddrSpace *as) {
  PTE *updir = (PTE *)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as) {}

void __am_get_cur_as(Context *c) {
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

void __am_switch(Context *c) {
  if (vme_enable && c->pdir != NULL) {
    set_satp(c->pdir);
  }
}

void map(AddrSpace *as, void *va, void *pa, int prot) {
  uintptr_t vaddr = (uintptr_t)va;
  uintptr_t paddr = (uintptr_t)pa;

  uint32_t vpn_1 = BITS(vaddr, 31, 22);
  uint32_t vpn_0 = BITS(vaddr, 21, 12);
  uint32_t ppn = BITS(paddr, 31, 12);

  PTE *updir = as->ptr;
  PTE *updir_pte = updir + vpn_1;

  if (*updir_pte == 0) {
    PTE *dir = (PTE *)(pgalloc_usr(PGSIZE));
    *updir_pte = (PTE)dir | PTE_V;
    PTE *pte = dir + vpn_0;
    *pte = ppn << 12 | PTE_V | PTE_X | PTE_W | PTE_R | PTE_U | PTE_D | PTE_A;
  } else {
    PTE *pte = (PTE *)((*updir_pte) & 0xFFFFF000) + vpn_0;
    *pte = ppn << 12 | PTE_V | PTE_X | PTE_W | PTE_R | PTE_U | PTE_D | PTE_A;
  }
}

Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
  Context *c = (Context *)(kstack.end - sizeof(Context));
  c->mstatus = 0;
  c->mepc = (uintptr_t)entry - 4;
  c->pdir = as->ptr;
  return c;
}

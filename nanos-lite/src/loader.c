#include "memory.h"
#include <elf.h>
#include <fs.h>
#include <proc.h>
#include <stdint.h>

#ifdef __LP64__
#define Elf_Ehdr Elf64_Ehdr
#define Elf_Phdr Elf64_Phdr
#else
#define Elf_Ehdr Elf32_Ehdr
#define Elf_Phdr Elf32_Phdr
#endif

#if defined(__ISA_AM_NATIVE__)
#define EXPECT_TYPE EM_X86_64
#elif defined(__ISA_X86__)
#define EXPECT_TYPE EM_386
#elif defined(__ISA_RISCV32__)
#define EXPECT_TYPE EM_RISCV
#else
#error Unsupported ISA
#endif

#define NR_PAGES 8

int fs_open(const char *pathname, int flags, int mode);
size_t fs_read(int fd, void *buf, size_t len);
size_t fs_lseek(int fd, size_t offset, int whence);
int fs_close(int fd);

static uintptr_t loader(PCB *pcb, const char *filename) {
  Elf_Ehdr ehdr;
  int fd = fs_open(filename, 0, 0);
  assert(fd >= 0);
  fs_read(fd, &ehdr, sizeof(ehdr));

  assert(ehdr.e_ident[0] == 0x7f && ehdr.e_ident[1] == 'E' &&
         ehdr.e_ident[2] == 'L' && ehdr.e_ident[3] == 'F');
  assert(ehdr.e_machine == EXPECT_TYPE);

  Elf_Phdr phdr[ehdr.e_phnum];
  fs_lseek(fd, ehdr.e_phoff, SEEK_SET);
  fs_read(fd, phdr, ehdr.e_phnum * ehdr.e_phentsize);

  uintptr_t max_brk = 0;

  for (int i = 0; i < ehdr.e_phnum; i++) {
    if (phdr[i].p_type == PT_LOAD) {
      uintptr_t va = phdr[i].p_vaddr & ~(PGSIZE - 1);
      int nr_pages = (phdr[i].p_memsz - 1) / PGSIZE + 1;
      printf("%d\n", nr_pages);
      void *pages = new_page(nr_pages);

      fs_lseek(fd, phdr[i].p_offset, SEEK_SET);
      fs_read(fd, pages, phdr[i].p_filesz);
      memset(pages + phdr[i].p_filesz, 0, phdr[i].p_memsz - phdr[i].p_filesz);

      for (int j = 0; j < nr_pages; j++) {
        map(&pcb->as, (void *)va + j * PGSIZE, pages + j * PGSIZE, 0b1110);
      }
      max_brk = max_brk > phdr[i].p_vaddr + phdr[i].p_memsz
                    ? max_brk
                    : phdr[i].p_vaddr + phdr[i].p_memsz;
    }
  }
  fs_close(fd);

  pcb->max_brk = (max_brk + PGSIZE - 1) & ~(PGSIZE - 1);
  return ehdr.e_entry;
}

void context_uload(PCB *pcb, const char *filename, char *const argv[],
                   char *const envp[]) {
  protect(&pcb->as);
  int argc = 0, envc = 0;
  void *stack_end = new_page(NR_PAGES) + NR_PAGES * PGSIZE; // ustack.end
  void *sp = stack_end;
  for (int i = 0; i < NR_PAGES; i++) {
    map(&pcb->as, pcb->as.area.end - (i + 1) * PGSIZE, sp - (i + 1) * PGSIZE,
        0b1110);
  }

  if (argv) {
    while (argv[argc]) {
      argc++;
    }
  }
  if (envp) {
    while (envp[envc]) {
      envc++;
    }
  }

  for (int i = envc - 1; i >= 0; i--) {
    sp -= strlen(envp[i]) + 1;
    strcpy((char *)sp, envp[i]);
  }
  for (int i = argc - 1; i >= 0; i--) {
    sp -= strlen(argv[i]) + 1;
    strcpy((char *)sp, argv[i]);
  }
  sp -= strlen(filename) + 1;
  strcpy((char *)sp, filename);

  uintptr_t *up = sp; // start of string area
  sp -= argc + envc + 4;
  *(int *)sp = argc;
  for (int i = 0; i < argc + 1; i++) {
    *(char **)(sp + 4 + i * 4) = (char *)up;
    up += strlen((const char *)up) + 1;
  }
  *(int *)(sp + 4 + argc * 4) = envc;
  for (int i = 0; i < envc; i++) {
    *(char **)(sp + 4 + (argc + 1 + i) * 4) = (char *)up;
    up += strlen((const char *)up) + 1;
  }
  *(int *)(sp + 4 + (argc + envc + 1) * 4) = 0;

  uintptr_t entry = loader(pcb, filename);
  Area kstack = {pcb->stack, pcb->stack + STACK_SIZE};
  pcb->cp = ucontext(&pcb->as, kstack, (void *)entry);
  pcb->cp->GPRx = (uintptr_t)(pcb->as.area.end - (stack_end - sp));
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void (*)())entry)();
}

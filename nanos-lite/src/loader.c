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

  for (int i = 0; i < ehdr.e_phnum; i++) {
    if (phdr[i].p_type == PT_LOAD) {
      fs_lseek(fd, phdr[i].p_offset, SEEK_SET);
      fs_read(fd, (void *)(uintptr_t)phdr[i].p_vaddr, phdr[i].p_filesz);

      memset((void *)(uintptr_t)(phdr[i].p_vaddr + phdr[i].p_filesz), 0,
             phdr[i].p_memsz - phdr[i].p_filesz);
    }
  }
  fs_close(fd);
  return ehdr.e_entry;
}

void context_uload(PCB *pcb, const char *filename, char *const argv[],
                   char *const envp[]) {
  uintptr_t entry = loader(pcb, filename);
  Area kstack = {pcb->stack, pcb->stack + STACK_SIZE};

  int argc = 0, envc = 0;
  uintptr_t *sp = new_page(NR_PAGES); // ustack.end

  if (argv) {
    while (argv[argc]) {
      printf("argv[%d]: %s\n", argc, argv[argc]);
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
  sp[0] = argc + 1;
  for (int i = 0; i < argc + 1; i++) {
    sp[1 + i] = (uintptr_t)up;
    up += strlen((const char *)up) + 1;
  }
  sp[2 + argc] = 0;
  for (int i = 0; i < envc; i++) {
    sp[3 + argc + i] = (uintptr_t)up;
    up += strlen((const char *)up) + 1;
  }
  sp[3 + argc + envc] = 0;

  pcb->cp = ucontext(NULL, kstack, (void *)entry);
  pcb->cp->GPRx = (uintptr_t)sp;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void (*)())entry)();
}

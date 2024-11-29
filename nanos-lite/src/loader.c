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
  Area ustack = {pcb->stack, pcb->stack + STACK_SIZE};
  pcb->cp = ucontext(NULL, ustack, (void *)entry);

  int argc = 0, envc = 0;
  uintptr_t *sp = heap.end;
  pcb->cp->GPRx = (uintptr_t)sp;

  while (argv[argc++])
    printf("%s\n", argv[argc]);
  while (*envp[envc++])
    ;

  printf("%s\n", argv);
  printf("%d %d\n", argc, envc);
  *(int *)sp++ = argc + envc;
  for (int i = 0; i < argc; i++) {
    *(char **)sp++ = argv[i];
  }
  *(char **)sp++ = NULL;

  for (int i = 0; i < envc; i++) {
    *(char **)sp++ = envp[i];
  }
  *(char **)sp++ = NULL;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void (*)())entry)();
}

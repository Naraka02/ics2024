#include "am.h"
#include <proc.h>
#include <elf.h>
#include <stdint.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t get_ramdisk_size();

static uintptr_t loader(PCB *pcb, const char *filename) {
  Elf32_Ehdr ehdr;
  ramdisk_read(&ehdr, 0, sizeof(ehdr));
  assert(ehdr.e_ident[0] == 0x7f && ehdr.e_ident[1] == 'E' && ehdr.e_ident[2] == 'L' && ehdr.e_ident[3] == 'F');

  Elf32_Phdr phdr[ehdr.e_phnum];
  ramdisk_read(phdr, ehdr.e_phoff, ehdr.e_phnum * ehdr.e_phentsize);

  uintptr_t ret = 0;
  for (int i = 0; i< ehdr.e_phnum; i++) {
    if (phdr[i].p_type == PT_LOAD) {
      ramdisk_read(&phdr[i].p_vaddr, phdr[i].p_offset, phdr[i].p_filesz);
      memset(&(phdr[i].p_vaddr) + phdr[i].p_filesz, 0, phdr[i].p_memsz - phdr[i].p_filesz);
      ret = phdr[i].p_vaddr;
    }
  }
  return ret;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}


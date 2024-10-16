#include <common.h>
#include <elf.h>

#define FUNCTAB_SIZE 1024

typedef struct {
  uint32_t addr;
  char *name;
} func;

func functab[FUNCTAB_SIZE];
int functab_size = 0;

void init_ftrace(const char *elf_file) {
  if (elf_file == NULL) {
    Log("No ELF file specified, skip initializing ftrace");
    return;
  }

  Log("Initializing ftrace with ELF file %s", elf_file);
  FILE *fp = fopen(elf_file, "rb");
  Assert(fp, "Failed to open ELF file %s", elf_file);

  Elf32_Ehdr ehdr;
  fread(&ehdr, sizeof(ehdr), 1, fp);

  Assert(ehdr.e_ident[0] == 0x7f && ehdr.e_ident[1] == 'E' && ehdr.e_ident[2] == 'L' && ehdr.e_ident[3] == 'F',
         "Invalid ELF file %s", elf_file);

  Elf32_Shdr shdr[ehdr.e_shnum];
  fseek(fp, ehdr.e_shoff, SEEK_SET);
  fread(shdr, sizeof(shdr), 1, fp);

  Elf32_Shdr symtab_shdr, strtab_shdr;
  for (int i = 0; i < ehdr.e_shnum; i++) {
    if (shdr[i].sh_type == SHT_SYMTAB) {
      symtab_shdr = shdr[i];
    }
    if (shdr[i].sh_type == SHT_STRTAB && i != ehdr.e_shstrndx) {
      strtab_shdr = shdr[i];
    }
  }

  Assert(symtab_shdr.sh_size % sizeof(Elf32_Sym) == 0, "Invalid symtab size");
  Assert(strtab_shdr.sh_size > 0, "Invalid strtab size");


  fseek(fp, symtab_shdr.sh_offset, SEEK_SET);
  Elf32_Sym symtab[symtab_shdr.sh_size / sizeof(Elf32_Sym)];
  fread(symtab, sizeof(symtab), 1, fp);

  for (int i = 0; i < symtab_shdr.sh_size / sizeof(Elf32_Sym); i++) {
    if (ELF32_ST_TYPE(symtab[i].st_info) == STT_FUNC) {
      char name[128];
      fseek(fp, strtab_shdr.sh_offset + symtab[i].st_name, SEEK_SET);
      fread(name, 128, 1, fp);
      functab[functab_size].addr = symtab[i].st_value;
      functab[functab_size].name = strdup(name);
      functab_size++;
    }
  }
  
  fclose(fp);
 
  for (int i = 0; i < functab_size; i++) {
    Log("Function %s at 0x%08x", functab[i].name, functab[i].addr);
  }
}


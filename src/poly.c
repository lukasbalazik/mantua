#include "mantua.h"


int load_text_section()
{
    int i;
    char *start = (char *)&_start;
    char *init = (char *)&_init;
    char *etext = (char *)&__etext;

    int j = start - init + 0x2000;
    printf("start: %p, init: %p, etext: %p\n", start, init, etext);

    for (i = 0; i < 30; i++) {
        printf("%x: %hhx\n", j+i, *(start+i));
    }

    return 0;
}

int rewrite_text_section(char *elf_fname)
{
    int elf_fd;
    int res;
    char *start = (char *)&_start;
    char *init = (char *)&_init;
    char *etext = (char *)&__etext;

    int i = start - init + 0x2000;

    elf_fd = open(elf_fname, O_RDWR);

    if (elf_fd < 0) {
        perror("Error: ");
        return elf_fd;
    }

    res = lseek(elf_fd, i, SEEK_SET);
    return 0;
}

#include "mantua.h"

int load_text_section() {
    int i;
    char *main = (char *)&main;
    char *init = (char *)&_init;

    int j = 0x2390;

    for (i = 0; i < 30; i++) {
        printf("%x: %hhx\n", j+i, *(main+i));
    }

    return 0;
}

int rewrite_text_section(char *elf_fname) {
    int elf_fd;
    int res;
    char *text = (char *)&text;
    char *init = (char *)&_init;

    int i = text - init + 0x2000;

    text += 4;
    change_page_permissions_of_address(text);
    elf_fd = open(elf_fname, O_RDWR);

    if (elf_fd < 0) {
        perror("Error: ");
        return elf_fd;
    }

    res = lseek(elf_fd, i+4, SEEK_SET);
    return 0;
}

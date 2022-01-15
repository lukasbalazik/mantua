#include "mantua.h"


int start_morphing(char *elf_fname)
{
    int elf_fd;
    char *s;
    char *section_bytes;
    size_t shstrndx;
    int ret = 0;

    elf_data_t elf;
    Elf_Scn *scn;
    GElf_Shdr shdr;


    elf_fd = open(elf_fname, O_RDWR);

    if (elf_fd < 0) {
        goto fail;
    }

    elf.fd = elf_fd;
    elf.e = NULL;

    if (elf_version(EV_CURRENT) == EV_NONE) {
        goto fail;
    }

    elf.e = elf_begin(elf.fd, ELF_C_READ, NULL);
    if (!elf.e) {
        goto fail;
    }


    if (elf_getshdrstrndx(elf.e, &shstrndx) < 0) {
        goto fail;
    }

    scn = NULL;
    while ((scn = elf_nextscn(elf.e, scn))) {
        if (!gelf_getshdr(scn, &shdr)) {
            goto fail;
        }
        s = elf_strptr(elf.e, shstrndx, shdr.sh_name);
        if (!s) {
            goto fail;
        }

        if (!strcmp(s, ".text")) {
          section_bytes = load_text_section(elf_fd, shdr.sh_offset, shdr.sh_size);
          section_bytes = morph_text_section(&shdr.sh_size, section_bytes);
          rewrite_text_section(elf_fd, shdr.sh_offset, shdr.sh_size, section_bytes);
        }
        // move other sections
    }
    if (!scn) {
        goto fail;
    }

    goto cleanup;

fail:
    ret = -1;

cleanup:
    if (elf.e) {
        elf_end(elf.e);
    }

    return ret;
}

char *load_text_section(int elf_fd, unsigned long int offset, unsigned long int size)
{
    char *bytes;

    bytes = (char *) malloc(size);

    lseek(elf_fd, offset, SEEK_SET);
    read(elf_fd, bytes, size);

    int i;
    for (i = 0; i < size; i++) {
        if (*(bytes + i) == '\xb8') {
            printf("%p - %02x\n", offset + i, *(bytes + i) & 0xff);
            i += 3;
        }
    }

    return bytes;
}

char *morph_text_section(unsigned long int *size, char *bytes)
{

    return NULL;
}

int rewrite_text_section(int elf_fd, unsigned long int offset, unsigned long int size, char *bytes)
{
    int res;

    res = lseek(elf_fd, offset, SEEK_SET);
    return res;
}

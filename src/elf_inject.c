#include "elf_inject.h"


int write_code(elf_data_t *elf, inject_data_t *inject)
{
    off_t off;
    size_t n;

    off = lseek(elf->fd, 0, SEEK_END);
    if (off < 0) {
        return -1;
    }

    n = write(elf->fd, inject->code, inject->len);
    if (n != inject->len) {
        return -1;
    }
    inject->off = off;

    return 0;
}


int write_ehdr(elf_data_t *elf)
{
    off_t off;
    size_t n, ehdr_size;
    void *ehdr_buf;

    if (!gelf_update_ehdr(elf->e, &elf->ehdr)) {
        return -1;
    }

    if (elf->bits == 32) {
        ehdr_buf = elf32_getehdr(elf->e);
        ehdr_size = sizeof(Elf32_Ehdr);
    } else {
        ehdr_buf = elf64_getehdr(elf->e);
        ehdr_size = sizeof(Elf64_Ehdr);
    }

    if (!ehdr_buf) {
        return -1;
    }

    off = lseek(elf->fd, 0, SEEK_SET);
    if (off < 0) {
        return -1;
    }

    n = write(elf->fd, ehdr_buf, ehdr_size);
    if (n != ehdr_size) {
        return -1;
    }

    return 0;
}


int write_phdr(elf_data_t *elf, inject_data_t *inject)
{
    off_t off;
    size_t n, phdr_size;
    Elf32_Phdr *phdr_list32;
    Elf64_Phdr *phdr_list64;
    void *phdr_buf;

    if (!gelf_update_phdr(elf->e, inject->pidx, &inject->phdr)) {
        return -1;
    }

    phdr_buf = NULL;
    if (elf->bits == 32) {
        phdr_list32 = elf32_getphdr(elf->e);
        if (phdr_list32) {
            phdr_buf = &phdr_list32[inject->pidx];
            phdr_size = sizeof(Elf32_Phdr);
        }
    } else {
        phdr_list64 = elf64_getphdr(elf->e);
        if (phdr_list64) {
            phdr_buf = &phdr_list64[inject->pidx];
            phdr_size = sizeof(Elf64_Phdr);
        }
    }
    if (!phdr_buf) {
        return -1;
    }

    off = lseek(elf->fd, elf->ehdr.e_phoff + inject->pidx*elf->ehdr.e_phentsize, SEEK_SET);
    if (off < 0) {
        return -1;
    }

    n = write(elf->fd, phdr_buf, phdr_size);
    if (n != phdr_size) {
        return -1;
    }

    return 0;
}


int write_shdr(elf_data_t *elf, Elf_Scn *scn, GElf_Shdr *shdr, size_t sidx)
{
    off_t off;
    size_t n, shdr_size;
    void *shdr_buf;

    if (!gelf_update_shdr(scn, shdr)) {
        return -1;
    }

    if (elf->bits == 32) {
        shdr_buf = elf32_getshdr(scn);
        shdr_size = sizeof(Elf32_Shdr);
    } else {
        shdr_buf = elf64_getshdr(scn);
        shdr_size = sizeof(Elf64_Shdr);
    }

    if (!shdr_buf) {
        return -1;
    }

    off = lseek(elf->fd, elf->ehdr.e_shoff + sidx*elf->ehdr.e_shentsize, SEEK_SET);
    if (off < 0) {
        return -1;
    }

    n = write(elf->fd, shdr_buf, shdr_size);
    if (n != shdr_size) {
        return -1;
    }

    return 0;
}


int reorder_shdrs(elf_data_t *elf, inject_data_t *inject)
{
    int direction, skip;
    Elf_Scn *scn;
    GElf_Shdr shdr;

    direction = 0;

    scn = elf_getscn(elf->e, inject->sidx - 1);
    if (scn && !gelf_getshdr(scn, &shdr)) {
        return -1;
    }

    if (scn && shdr.sh_addr > inject->shdr.sh_addr) {

        direction = -1;
    }

    scn = elf_getscn(elf->e, inject->sidx + 1);
    if (scn && !gelf_getshdr(scn, &shdr)) {
        return -1;
    }

    if (scn && shdr.sh_addr < inject->shdr.sh_addr) {

        direction = 1;
    }

    if (direction == 0) {

        return 0;
    }


    skip = 0;
    for (scn = elf_getscn(elf->e, inject->sidx + direction);
            scn != NULL;
            scn = elf_getscn(elf->e, inject->sidx + direction + skip)) {

        if (!gelf_getshdr(scn, &shdr)) {
            return -1;
        }

        if ((direction < 0 && shdr.sh_addr <= inject->shdr.sh_addr)
             || (direction > 0 && shdr.sh_addr >= inject->shdr.sh_addr)) {

            break;
        }


        if (shdr.sh_type != SHT_PROGBITS) {
            skip += direction;
            continue;
        }


        if (write_shdr(elf, scn, &inject->shdr, elf_ndxscn(scn)) < 0) {
            return -1;
        }

        if (write_shdr(elf, inject->scn, &shdr, inject->sidx) < 0) {
            return -1;
        }

        inject->sidx += direction + skip;
        inject->scn = elf_getscn(elf->e, inject->sidx);
        skip = 0;
    }

    return 0;
}


int write_secname(elf_data_t *elf, inject_data_t *inject)
{
    off_t off;
    size_t n;

    off = lseek(elf->fd, inject->shstroff, SEEK_SET);
    if (off < 0) {
        return -1;
    }

    n = write(elf->fd, inject->secname, strlen(inject->secname));
    if (n != strlen(inject->secname)) {
        return -1;
    }

    n = strlen(ABITAG_NAME) - strlen(inject->secname);
    while (n > 0) {
        if (!write(elf->fd, "\0", 1)) {
            return -1;
        }
        n--;
    }

    return 0;
}


int find_rewritable_segment(elf_data_t *elf, inject_data_t *inject)
{
    int ret;
    size_t i, n;

    ret = elf_getphdrnum(elf->e, &n);
    if (ret != 0) {
        return -1;
    }

    for (i = 0; i < n; i++) {
        if (!gelf_getphdr(elf->e, i, &inject->phdr)) {
            return -1;
        }

        switch (inject->phdr.p_type) {
        case PT_NOTE:
            inject->pidx = i;
            return 0;
        default:
            break;
        }
    }

    return -1;
}


int rewrite_code_segment(elf_data_t *elf, inject_data_t *inject)
{
    inject->phdr.p_type = PT_LOAD;
    inject->phdr.p_offset = inject->off;
    inject->phdr.p_vaddr = inject->secaddr;
    inject->phdr.p_paddr = inject->secaddr;
    inject->phdr.p_filesz = inject->len;
    inject->phdr.p_memsz = inject->len;
    inject->phdr.p_flags = PF_R | PF_X;
    inject->phdr.p_align = 0x1000;

    if (write_phdr(elf, inject) < 0) {
        return -1;
    }

    return 0;
}


int rewrite_code_section(elf_data_t *elf, inject_data_t *inject)
{
    Elf_Scn *scn;
    GElf_Shdr shdr;
    char *s;
    size_t shstrndx;

    if (elf_getshdrstrndx(elf->e, &shstrndx) < 0) {
        return -1;
    }

    scn = NULL;
    while ((scn = elf_nextscn(elf->e, scn))) {
        if (!gelf_getshdr(scn, &shdr)) {
            return -1;
        }
        s = elf_strptr(elf->e, shstrndx, shdr.sh_name);
        if (!s) {
            return -1;
        }

        if (!strcmp(s, ABITAG_NAME)) {
            shdr.sh_name            = shdr.sh_name;
            shdr.sh_type            = SHT_PROGBITS;
            shdr.sh_flags         = SHF_ALLOC | SHF_EXECINSTR;
            shdr.sh_addr            = inject->secaddr;
            shdr.sh_offset        = inject->off;
            shdr.sh_size            = inject->len;
            shdr.sh_link            = 0;
            shdr.sh_info            = 0;
            shdr.sh_addralign = 16;
            shdr.sh_entsize     = 0;

            inject->sidx = elf_ndxscn(scn);
            inject->scn = scn;
            memcpy(&inject->shdr, &shdr, sizeof(shdr));

            if (write_shdr(elf, scn, &shdr, elf_ndxscn(scn)) < 0) {
                return -1;
            }

            if (reorder_shdrs(elf, inject) < 0) {
                return -1;
            }

            break;
        }
    }
    if (!scn) {
        return -1;
    }

    return 0;
}


int rewrite_section_name(elf_data_t *elf, inject_data_t *inject)
{
    Elf_Scn *scn;
    GElf_Shdr shdr;
    char *s;
    size_t shstrndx, stroff, strbase;

    if (strlen(inject->secname) > strlen(ABITAG_NAME)) {
        return -1;
    }

    if (elf_getshdrstrndx(elf->e, &shstrndx) < 0) {
        return -1;
    }

    stroff = 0;
    strbase = 0;
    scn = NULL;
    while ((scn = elf_nextscn(elf->e, scn))) {
        if (!gelf_getshdr(scn, &shdr)) {
            return -1;
        }
        s = elf_strptr(elf->e, shstrndx, shdr.sh_name);
        if (!s) {
            return -1;
        }

        if (!strcmp(s, ABITAG_NAME)) {
            stroff = shdr.sh_name;
        } else if (!strcmp(s, SHSTRTAB_NAME)) {
            strbase = shdr.sh_offset;
        }
    }

    if (stroff == 0) {
        return -1;
    } else if (strbase == 0) {
        return -1;
    }

    inject->shstroff = strbase + stroff;

    if (write_secname(elf, inject) < 0) {
        return -1;
    }

    return 0;
}



int inject_code(int fd, inject_data_t *inject)
{
    elf_data_t elf;
    int ret;
    size_t n;

    elf.fd = fd;
    elf.e = NULL;

    if (elf_version(EV_CURRENT) == EV_NONE) {
        goto fail;
    }


    elf.e = elf_begin(elf.fd, ELF_C_READ, NULL);
    if (!elf.e) {
        goto fail;
    }

    if (elf_kind(elf.e) != ELF_K_ELF) {
        goto fail;
    }

    ret = gelf_getclass(elf.e);
    switch (ret) {
        case ELFCLASSNONE:
            goto fail;
        case ELFCLASS32:
            elf.bits = 32;
            break;
        default:
            elf.bits = 64;
            break;
    }

    if (!gelf_getehdr(elf.e, &elf.ehdr)) {
        goto fail;
    }


    if (find_rewritable_segment(&elf, inject) < 0) {
        goto fail;
    }


    if (write_code(&elf, inject) < 0) {
        goto fail;
    }

    n = (inject->off % 4096) - (inject->secaddr % 4096);
    inject->secaddr += n;
    ret = inject->secaddr;

    if ((rewrite_code_section(&elf, inject) < 0)
            || (rewrite_section_name(&elf, inject) < 0)) {
        goto fail;
    }

    if (rewrite_code_segment(&elf, inject) < 0) {
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

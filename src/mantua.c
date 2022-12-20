#include "mantua.h"

 void (*error_handler)();
 int have_section;

void mantua_init()
{
    char *init = (char *)&__executable_start;
    change_page_permissions_of_address(init);
    auto_time_capsule_position = 0;
    read_persistent_time_storage();
}

void mantua_finish(char *file)
{
    if (!have_section && auto_time_capsule_position) {
        int res = create_persistent_time_storage(file);
        if (res != -1)
            rewrite_create_persistent_time_storage(file, res);
    }
}

int change_page_permissions_of_address(void *addr)
{
    int page_size = getpagesize();
    addr -= (unsigned long)addr % page_size;
    if (mprotect(addr, page_size*10, PROT_READ | PROT_WRITE | PROT_EXEC) == -1) {
        return -1;
    }
    return 0;
}


void *create_shared_memory(size_t size)
{
  int protection = PROT_READ | PROT_WRITE;

  int visibility = MAP_SHARED | MAP_ANONYMOUS;

  return mmap(NULL, size, protection, visibility, -1, 0);
}


int read_persistent_time_storage()
{
    int i = 0;
    char base[] = "\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90";

    char *control = (char *)&create_persistent_time_storage;
    int res = -1;
    control += 12;
    for (i = 0; i < 10; i++)
        if (*(base + i) != *(control + i)) {
            res = 1;
            break;
        }

    if (res > 0)
        return -1;

    control -= 8;

    have_section = 1;

    i = *(int *)control;
    control += 4;
    capsule_count = *(int *)control;

    char *address = (char *)&__executable_start;
    address += i;

    struct capsule *c = (struct capsule *)address;
    cap_ptr = (struct capsule *) realloc(cap_ptr, capsule_count * sizeof(struct capsule));
    for (i = 0; i < capsule_count; i++) {
        struct capsule cap = *(c+i);
        *(cap_ptr + i) = cap;
    }

    return 1;
}

int rewrite_create_persistent_time_storage(char *elf_fname, int storage)
{
    int elf_fd;
    int res;
    char buff[] = "\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90";
    char *address = (char *)&create_persistent_time_storage;
    char *init = (char *)&__executable_start;

    int i = address - init;

    address += 4;
    change_page_permissions_of_address(address);
    elf_fd = open(elf_fname, O_RDWR);

    if (elf_fd < 0) {
        perror("Error: ");
        return elf_fd;
    }

    res = lseek(elf_fd, i+4, SEEK_SET);
    res = write(elf_fd, (void *)&storage, 4);
    res = write(elf_fd, (void *)&capsule_count, 4);
    for (i = 0; i < 0x13; i++) {
        res = write(elf_fd, buff, 10);
    }
    return res;
}

int create_persistent_time_storage(char *elf_fname)
{
    int elf_fd, ret;
    inject_data_t inject;

    elf_fd = open(elf_fname, O_RDWR);
    if (elf_fd < 0) {
        perror("Error: ");
        return elf_fd;
    }

    inject.code = (char *)cap_auto_ptr;
    inject.len = auto_time_capsule_position*sizeof(struct capsule);
    inject.secname = "time_storage";
    inject.secaddr = 8388608;

    ret = inject_code(elf_fd, &inject);

    close(elf_fd);

    return ret;
}

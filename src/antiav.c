#include "mantua.h"

int total_size;
int f_size;
struct crypter_functions *functions;

char *rand_string(size_t size) {
    char *str;
    str = malloc(size);
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    if (size) {
        --size;
        for (size_t n = 0; n < size; n++) {
            int key = rand() % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
    }
    return str;
}

size_t function_size(void *addr_arg) {
    char *addr = (char *)addr_arg;
    size_t function_sz = 0;
    while(*addr != 0xc3 && *addr != 0xc2 && *(addr + 1) != 0x55) {
          function_sz++;
          addr++;
    }
    return function_sz + 1;
}

void crypter_set_function(void *function)
{
    if (f_size < 1)
        srand(time(0));

    int size = function_size(function);

    if (!functions) {
        functions = malloc(sizeof(struct crypter_functions));
        f_size = 1;
    } else {
        f_size++;
        functions = realloc(functions, f_size*(sizeof(struct crypter_functions)));
    }


    (functions + f_size - 1)->function = function;
    (functions + f_size - 1)->size = size;
    (functions + f_size - 1)->key = malloc(16);
    (functions + f_size - 1)->iv = malloc(16);

    if (!is_encrypted(function)) {
        memcpy((functions + f_size - 1)->key, rand_string(16), 16);
        memcpy((functions + f_size - 1)->iv, rand_string(16), 16);
        char *buff = read_and_encrypt(function, (functions + f_size - 1)->key, (functions + f_size - 1)->iv);
        (functions + f_size - 1)->buff = buff;
    } else {
        char *address = (char *)(functions + f_size - 1)->function;
        int size = *(int *)(address + 4);
        if (!total_size)
            total_size = size + (16 - size % 16);
        else
            total_size += size + (16 - size % 16);
    }

}

void crypter_init(char *elf_name)
{
    int i;
    int j;
    int pos = 0;

    char *elf_start = (char *)&__executable_start;
    char *address = (char *)functions->function;
    int sec_addr = *(int *)address;

    if (is_encrypted(functions->function)) {
        for (i = 0; i < f_size; i++) {
            char key[16];
            char iv[16];

            for (j = 0; j < 16; j++) {
                key[j] = *(char *)(sec_addr + elf_start + total_size + pos + j);
                iv[j] = *(char *)(sec_addr + elf_start + total_size + pos + j + 16);
            }

            memcpy((functions + i)->key, key, 16);
            memcpy((functions + i)->iv, iv, 16);
            pos += 32;
        }
        decrypt_and_copy_functions(functions);
    } else {
        int sec_addr = add_as_section(elf_name, functions);
        rewrite_function(elf_name, functions, sec_addr);
        exit(1);
    }
}

int is_encrypted(void *function)
{
    int i = 0;
    char base[] = "\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90";

    char *addr = (char *)function;

    for (i = 0; i < 10; i++) {
        if (*(base + i) != *(addr + 8 + i)) {
            return 0;
        }
    }

    return 1;
}

char *read_and_encrypt(void *function, char *key, char *iv)
{
    int i;
    char *buff;
    int size = function_size(function);
    uint8_t *addr = encrypt_function((uint8_t *)function, size, (uint8_t *)key, (uint8_t *)iv);

    int new_size = size + (16 - size % 16);

    buff = (char *)malloc(new_size);

    for (i = 0; i < new_size; i++)
        *(buff + i) = *(addr + i);


    return buff;
}

uint8_t *encrypt_function(uint8_t *addr_buff, int size, uint8_t *key, uint8_t *iv)
{
    uint8_t *buff;

    int new_size = size + (16 - size % 16);

    buff = malloc(new_size);
    memset(buff, 0, new_size);

    int i;
    for (i = 0; i < size; i++)
        *(buff + i) = *(addr_buff + i);

    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, iv);
    AES_CBC_encrypt_buffer(&ctx, buff, size);
    return buff;
}

int add_as_section(char *elf_fname, struct crypter_functions *functions)
{
    int i;
    int ret;
    int elf_fd;
    int size;
    int increase = 0;
    char *buff = NULL;

    inject_data_t inject;


    elf_fd = open(elf_fname, O_RDWR);
    if (elf_fd < 0) {
        perror("Error: ");
        return -1;
    }

    for (i = 0; i < f_size; i++) {
        size = (functions + i)->size;
        size = size + (16 - size % 16);
        increase += size;

        if (!buff) {
            buff = malloc(increase);
        } else {
            buff = realloc(buff, increase);
        }

        memcpy(buff + increase - size, (functions + i)->buff, size);
    }

    for (i = 0; i < f_size; i++) {
        buff = realloc(buff, increase + 32);
        memcpy(buff + increase, (functions + i)->key, 16);
        memcpy(buff + increase + 16, (functions + i)->iv, 16);
        increase += 32;
    }


    inject.code = buff;
    inject.len = increase;
    inject.secname = "test";
    inject.secaddr = 4389608;

    ret = inject_code(elf_fd, &inject);

    close(elf_fd);
    return ret;
}

uint8_t *decrypt_function(uint8_t *addr_buff, int size, uint8_t *key, uint8_t *iv)
{
    uint8_t *buff;

    int new_size = size + (16 - size % 16);

    buff = malloc(new_size);
    memset(buff, 0, new_size);

    int i;
    for (i = 0; i < new_size; i++)
        *(buff + i) = *(addr_buff + i);

    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key, iv);
    AES_CBC_decrypt_buffer(&ctx, buff, size);
    return buff;
}

void decrypt_and_copy_functions(struct crypter_functions *functions)
{
    int i;
    int j;

    char *elf_start = (char *)&__executable_start;


    for (j = 0; j < f_size; j++) {
        char *address = (char *)(functions + j)->function;
        change_page_permissions_of_address(address);

        int sec_addr = *(int *) address;
        int size = *(int *)(address + 4);

        uint8_t *key = (uint8_t *)(functions + j)->key;
        uint8_t *iv = (uint8_t *)(functions + j)->iv;


        uint8_t *buff = decrypt_function((uint8_t *)(sec_addr + elf_start), size, key, iv);

        for (i = 0; i < size; i++) {
            *(address + i) = *(buff + i);
        }
    }

}

void rewrite_function(char *elf_fname, struct crypter_functions *functions, int sec_addr)
{
    int i;
    int elf_fd;
    char buff[] = "\x90";
    char *elf_start = (char *)&__executable_start;

    elf_fd = open(elf_fname, O_RDWR);

    if (elf_fd < 0) {
        perror("Error: ");
        return;
    }

    for (i = 0; i < f_size; i++) {
        char *address = (char *)(functions + i)->function;
        int size = (functions + i)->size;
        int new_size = size + (16 - size % 16);

        int p = address - elf_start;

        lseek(elf_fd, p, SEEK_SET);
        write(elf_fd, (void *)&sec_addr, 4);
        write(elf_fd, (void *)&size, 4);

        for (p = 0; p < size - 8; p++) {
            write(elf_fd, buff, 1);
        }
        sec_addr += new_size;
    }
}

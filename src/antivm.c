#include "mantua.h"

int hypervisor_in_cpuinfo() {
    int num_read;
    int cpuinfo_fd;
    char buf[4096];
    char hypervisor_string[] = "hypervisor";

    int ret = 0;

    cpuinfo_fd  = open("/proc/cpuinfo", O_RDONLY);
    if (cpuinfo_fd == -1)
        return -1;

    do {
        num_read = read(cpuinfo_fd, buf, sizeof(buf) - 1);
        buf[num_read] = '\0';
        if (strstr(buf, hypervisor_string)) {
            ret = 1;    
        }
    } while (num_read <= 0);

    if (ret != 0) {
        BREAK_EVERYTHING();
    }

    return ret;
}

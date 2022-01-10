#include "mantua.h"

int hypervisor_in_cpuinfo()
{
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

int vm_signs_in_klog()
{
    int len;
    char *buf;
    int in = 0;

    char strings[7][50] = {
                    "VirtualBox",
                    "virtualbox",
                    "VMWARE",
                    "VMware",
                    "KVM",
                    "Hypervisor detected",
                    "vboxsf"
    };


    len = klogctl(10, NULL, 0);
    buf = malloc(len);
    len = klogctl(3, buf, len);

    if (len == 0) {
        BREAK_EVERYTHING();
        return 1;
    }

    for (in = 0; in < 7; in++) {
        char *line;
        line = strstr(buf, *(strings + in));
        if (line) {
            BREAK_EVERYTHING();
            return 1;
        }

    }

    return 0;
}

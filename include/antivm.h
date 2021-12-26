#ifndef ANTIVM_H
#define ANTIVM_H

#include <sys/klog.h>

int hypervisor_in_cpuinfo();
int vm_signs_in_klog();

#endif

#include <stdio.h>
#include "mantua.h"


int main(int argc, char *argv[])  {
    mantua_init();

    int tracer = tracer_pid();
    if (tracer < 0)
        printf("Im traced by process %d\n", tracer);

    block_tracing();

    int i = count_0xcc();
    printf("Number of 0xcc in code: %d\n", i);

    int cap1 = start_time_capsule(1500000);

    printf("Sleep for 1 second\n");
    sleep(1);

    int autocap1 = start_time_capsule(AUTO_TIME);

    printf("Im in AUTO_TIME capsule \n");
    if (stop_time_capsule(autocap1) < 0)
        printf("AUTO_TIME capsule taken longer\n");


    if (stop_time_capsule(cap1) < 0)
        printf("Capsule 1 Taken Longer than 1500000 microseconds\n");

    printf("My code that will not be debbuged\n");
    fflush(stdout);

    int ret = hypervisor_in_cpuinfo();
    printf("hypervisor in /proc/cpuinfo: %d\n", ret);
    
    ret = vm_signs_in_klog();
    printf("vm_signs_in_klog: %d\n", ret);

    mantua_finish(argv[0]);

    return 0;
}

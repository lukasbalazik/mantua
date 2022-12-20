#include <stdio.h>
#include "mantua.h"

int secret() {
    printf("Hello From Secret Function\n");
    printf("Our function Secret Should be now encrypted\n");
    return 0;
}


int secret2() {
    printf("Hello From Secret2 Function\n");
    printf("Our function Secret2 Should be now encrypted\n");
    return 0;
}

int main(int argc, char *argv[])
{
    crypter_set_function(secret);
    crypter_set_function(secret2);

    crypter_init(argv[0]);

    secret();
    secret2();
    mantua_init();

    int tracer = tracer_pid();
    if (tracer)
        printf("Im traced by process starting with %d\n", tracer);

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

    int ret = hypervisor_in_cpuinfo();
    printf("hypervisor in /proc/cpuinfo: %d\n", ret);


//    ret = vm_signs_in_klog();
//    printf("vm_signs_in_klog: %d\n", ret);

    mantua_finish(argv[0]);

//    if (start_morphing(argv[0]) < 0)
//        printf("Morphing failed :(\n");

    return 0;
}

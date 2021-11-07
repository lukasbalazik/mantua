#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <sys/ptrace.h>
#include <sys/wait.h>

#include <string.h>

int main(int argc, char *argv[]) {
    int pid = fork();
    if (pid == 0) {
        char *binaryPath = "../build/test";
        execl(binaryPath, binaryPath, NULL, NULL, NULL);
    } else {
    int i = -1;
    int j = 0;
    while (i < 0) {
        i = ptrace(PTRACE_SEIZE, pid, NULL, NULL);
        j++;
    }
    
    printf("output %d with %d iterations\n", i,j);
    sleep(300);
    }
}

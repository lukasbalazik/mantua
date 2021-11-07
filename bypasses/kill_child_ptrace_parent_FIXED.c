#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <sys/ptrace.h>
#include <sys/wait.h>

#include <string.h>

int main(int argc, char *argv[]) {
    int pid_of_parent = atoi(argv[2]);
    int pid_of_child = atoi(argv[1]);
    kill(pid_of_child, SIGKILL);
    int i = -1;
    int j = 0;
    while (i < 0) {
        i = ptrace(PTRACE_SEIZE, pid_of_parent, NULL, NULL);
        j++;
    }
    printf("tracing %d with %d itterations\n", i,j);
    sleep(3000);
}

#include "mantua.h"

#include <sys/ptrace.h>
#include <sys/wait.h>

atomic_int acnt;
int capsule_count;
int auto_time_capsule_position;
struct capsule *cap_ptr;
struct capsule *cap_auto_ptr;

int count_0xcc()
{
    int count = 0;
    char *start = (char *)&_start;
    char *end = (char *)&__etext;

    volatile unsigned char bppart[1] = { 0x66 };

    while (start != end) {
        if (((*(volatile unsigned *)start) & 0xFF) == (*bppart) + (*bppart)) {
            count++;
        }
        ++start;
    }
    return count;
}

int tracer_pid()
{
    char buf[4096];
    int num_read;
    int status_fd;
    char tracer_pid_string[] = "TracerPid:";
    char tracer_pid_ptr[4096];
    char *c_ptr;
    int ret = -1;

    status_fd  = open("/proc/self/status", O_RDONLY);
    if (status_fd == -1)
        return -1;

    num_read = read(status_fd, buf, sizeof(buf) - 1);
    if (num_read <= 0)
        return -1;

    buf[num_read] = '\0';
    strncpy(tracer_pid_ptr, strstr(buf, tracer_pid_string) + 11, num_read);

    for (c_ptr = tracer_pid_ptr + sizeof(tracer_pid_string) - 1; c_ptr <= tracer_pid_ptr + num_read; ++c_ptr) {
        ret = isdigit(*c_ptr) != 0 && *c_ptr != '0';
        if (ret)
            break;
    }

    if (ret != 0) {
        BREAK_EVERYTHING();
    }

    return ret;
}

int start_time_capsule(uint64_t microseconds)
{
    if (microseconds == AUTO_TIME && have_section) {
        if (!(cap_ptr + auto_time_capsule_position)) {
            BREAK_EVERYTHING();
            return -1;
        }
        gettimeofday(&(cap_ptr + auto_time_capsule_position)->start, NULL);
        return auto_time_capsule_position++;
    }

    if (microseconds == AUTO_TIME) {
        auto_time_capsule_position++;
    }

    struct capsule cap;

    if (!capsule_count)
        capsule_count = 0;

    capsule_count++;
    cap_ptr = (struct capsule *) realloc(cap_ptr, capsule_count * sizeof(struct capsule));
    cap.id = capsule_count - 1;
    cap.microseconds = microseconds;
    gettimeofday(&cap.start, NULL);
    *(cap_ptr + cap.id) = cap;

    return cap.id;
}

int stop_time_capsule(int capsule_id)
{
    int ret = 0;
    if (!(cap_ptr + capsule_id) || capsule_id < 0) {
        BREAK_EVERYTHING();
        return -1;
    }

    struct capsule *cap = (cap_ptr + capsule_id);

    gettimeofday(&cap->end, NULL);
    uint64_t delta_us = (cap->end.tv_sec - cap->start.tv_sec) * 1000000 + cap->end.tv_usec - cap->start.tv_usec;

    if (delta_us < 0) {
        BREAK_EVERYTHING();
    }

    if (delta_us > cap->microseconds)
        ret = -1;

    if (!have_section && cap->microseconds == AUTO_TIME) {
        if (!delta_us)
            delta_us = 1;
        if (delta_us < 10) {
            delta_us *= 20;
        } else if (delta_us < 100) {
            delta_us *= 4;
        } else if (delta_us < 1000) {
            delta_us *= 2;
        } else {
            delta_us = (int)((float)delta_us * 1.3);
        }

        cap->microseconds = delta_us;
        cap->id = auto_time_capsule_position - 1;
        cap_auto_ptr = (struct capsule *) realloc(cap_auto_ptr, auto_time_capsule_position * sizeof(struct capsule));
        *(cap_auto_ptr + auto_time_capsule_position - 1) = *cap;
    }

    if (auto_time_capsule_position != capsule_count && !have_section) {
        capsule_count--;
        cap_ptr = (struct capsule *) realloc(cap_ptr, capsule_count * sizeof(struct capsule));
    }


    return ret;
}


int ptrace_seize(int pid, int type, int *fd)
{
    int status;
    int res = -1;
    int val = 1;

    if (ptrace(PTRACE_SEIZE, pid, NULL, NULL) == 0) {
        if (type == CHILD)
            res = write(fd[1], &val, sizeof(val));

        do {
            waitpid(pid, &status, 0);
            ptrace(PTRACE_CONT, pid, NULL, NULL);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));

        ptrace(PTRACE_DETACH, pid, NULL, NULL);
        res = status;
    }

    if (type == PARENT && status < 32) {
        sleep(0.001);
        BREAK_EVERYTHING();
    }
    return res;
}

void *ptrace_cycle(void *var)
{
    int fd[2];
    int val = 0;
    int res = 0;

    res = pipe(fd);

    int pid = fork();

    if (pid == -1) {
        BREAK_EVERYTHING();
    }

    if (pid == 0) {

        int ppid = getppid();
        close(fd[0]);
        res = ptrace_seize(ppid, CHILD, fd);
        close(fd[1]);
        exit(res);

    } else {
        close(fd[1]);
        int flags = fcntl(fd[0], F_GETFL, 0);
        fcntl(fd[0], F_SETFL, flags | O_NONBLOCK);

        int retry;
        for (retry = 0; retry < 1000; retry++) {
            sleep(0.001);
            res = read(fd[0], &val, sizeof(val));
            if (val)
                break;
        }

        if (!val) {
            fflush(stdout);
            BREAK_EVERYTHING();
        }
        acnt = 1;

        ptrace_seize(pid, PARENT, fd);
        close(fd[0]);
    }
    return NULL;
}


int block_tracing()
{
    int retry = 0;
    pthread_t tid;
    pthread_create(&tid, NULL, ptrace_cycle, NULL);
    while (!acnt) {
        if (retry == 1100) {
            fflush(stdout);
            BREAK_EVERYTHING();
        }
        sleep(0.001);
        retry++;
    }
    if (!acnt)
        return -1;
    return 0;
}

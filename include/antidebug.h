#ifndef ANTIDEBUG_H
#define ANTIDEBUG_H

#include <sys/time.h>

#define AUTO_TIME -1

int count_0xcc();
int tracer_pid();
int block_tracing();
int start_time_capsule(uint64_t);
int stop_time_capsule(int);

struct capsule {
   struct timeval start;
   struct timeval end;
   uint64_t microseconds;
   int id;
};

extern int capsule_count;
extern int auto_time_capsule_position;
extern struct capsule *cap_ptr;
extern struct capsule *cap_auto_ptr;

#endif

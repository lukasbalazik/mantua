# mantua

**mantua** (named after 8th circle of hell), is AntiDebug, AntiVM, AntiAV C library for linux.

## DISCLAIMER: FOR EDUCATIONAL AND INFORMATIONAL PURPOSES ONLY

This project is for malware analysts and for creator of CTF or cyber security exercises.

This project can be also used for your proprietary software to protect your Intellectual Property.

## Test program with all functionality

**./test/test.c** - is mine test file which is used in Makefile to test all functionality. If you use AUTO_TIME capsules, at your first start you have to use linker from user-space because if its served trough kernel you will get locking which prevent our binary rewrite, in my case:

```sh
sudo /lib64/ld-linux-x86-64.so.2 ./build/test
```

For finding right path use

```sh
ldd ./build/test
....
....
/lib64/ld-linux-x86-64.so.2 (0x00007f30ffcff000)
....
```

For all other runs you run it just by:

```sh
sudo ./build/test
```

* **sudo** if block_tracing() is used

### Bypasses

./bypasses/ directory contain some bypasses for mantua functionality, if its not possible anymore, it contain **_FIXED** in name

## Basic functionality

### BREAK_EVERYTHING() macro

Macro which contain rewrite of bytes from symbol **_init** with **0x90**. This functionality is triggered if we dont have **error_handler** function pointer defined. If we have error_handler, its called instead.

### mantua_init()

Function which initialize mantua library, change permissions of some addresses (for BREAK_EVERYTHING functionality), and read persistent time storage if we have one.

### mantua_finish(char *)

Function which at first run save **AUTO_TIME** time capsules into separate section and rewrite storing function. If you are using **AUTO_TIME** functionality call this function before your infinity loops, if any are used. Argument is **argv[0]** (program path and name)

## AntiDebug

For now, this library have 4 antidebug functionality.

### Counting INT3 in code (0xcc)

This functionality basically read it self from memory which means if the default value is different then value at runtime, your code have INT3 which is software breakpoint. Be careful, this function counts 0xcc which can be part of valid instruction so return is not zero every time (we can find this via **objdump -d | grep " cc"**), but in runtime it have still the same value if software breakpoints are not inserted

#### int count_0xcc();

Return value is number of 0xcc found from symbol **_start** to symbol **__etext**

---

### Reading Tracer pid from /proc/self/status 

Program read **/proc/self/status** and check if **TracerPid** is **0**, if not we call **BREAK_EVERYTHING()**. If you want to you use this functionality you have to run this function before **block_tracing()**

#### int tracer_pid();

return **0** or pid of process which is tracing us

---

### Parend <-> Child ptrace

We fork and trace our child, and child start tracing us, if this all takes more than second, we **BREAK_EVERYTHING()**. If the child is not created and we don't receive **1** from pipe within one second we also **BREAK_EVERYTHING()**. For this functionality we have to run our program with **root** privileges

#### int block_tracing();

we return **0** if forking and tracing was successfull, **-1** if not

---

### Time Capsules

Time capsules are control time between instructions. You can define this time, or it can be defined automatically.  With this you can prevent breakpointing in your code, because breakpoints will increase execution time.

Example of time capsule:

```c
int cap1 = start_time_capsule(1500);

printf("Im in capsule\n");
if (stop_time_capsule(cap1) < 0)
    printf("Capsule 1 taken Longer than 1500 microseconds\n");
```

or for automatic time:

```c
int autocap1 = start_time_capsule(AUTO_TIME);

printf("Im in AUTO_TIME capsule \n");
if (stop_time_capsule(autocap1) < 0)
    printf("AUTO_TIME capsule taken longer\n");
```

#### int start_time_capsule(uint64_t);

Starting capsule, argument is max time for capsule. In case of automatic time use macro **AUTO_TIME**. Function return ID of capsule.

#### int stop_time_capsule(int);

Stopping capsule, argument is ID of capsule and return is **0** if time was under defined time, or **-1** if your block of code taken longer

## AntiVM

Soon...

## AntiAV

Soon...




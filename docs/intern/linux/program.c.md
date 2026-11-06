# src/linux/program.c

On Linux, this file reads the link of the program in `/proc`, and counts the processors of the affinity of the process.

## The program

`os_get_program_path` reads the link `/proc/self/exe` with `readlink`, which writes no end of text.
The buffer starts with `PATH_MAX` bytes. If `readlink` fills it, the buffer doubles, and the read starts again.

## The processors

`os_get_processor_count` asks `sched_getaffinity` for the set of the processors of the process. `CPU_COUNT_S` counts them.
The set starts with room for 1024 processors. If `sched_getaffinity` fails with `EINVAL`, the set doubles, and the call starts again.
Thus `taskset` and the limits of a container change the count. Another failure gives 0.

## Functions

| Function | Result |
| --- | --- |
| `os_get_program_path()` | Gives the target of `/proc/self/exe`, or `nullptr`. |
| `os_get_processor_count()` | Gives `CPU_COUNT_S` of the affinity, or 0. |

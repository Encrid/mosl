# src/linux/program.c

On Linux, this file reads the link of the program in `/proc`, and counts the processors of the affinity of the process.

## The program

`os_get_program_path` reads the link `/proc/self/exe` with `readlink`, which writes no end of text.
The buffer starts with 16 bytes. If `readlink` fills it, the buffer doubles, and the read starts again.
Thus each call runs the growth, and the cases run each of its branches.

## The processors

`os_get_processor_count` asks `sched_getaffinity` for the set of the processors of the process, and `CPU_COUNT` counts them.
Thus `taskset` and the limits of a container change the count.
The set has room for 1024 processors. On a machine with more, `sched_getaffinity` fails.
Then the count is `os_get_online_count` of `src/posix/online.c`, as `available_parallelism` of Rust does.

## Functions

| Function | Result |
| --- | --- |
| `os_get_program_path()` | Gives the target of `/proc/self/exe`, or `nullptr`. |
| `os_get_processor_count()` | Gives `CPU_COUNT` of the affinity, or `os_get_online_count`. |

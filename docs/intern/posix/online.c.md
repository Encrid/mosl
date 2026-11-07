# src/posix/online.c

This file counts the processors that run, on Linux and macOS.

## The count

`os_get_online_count` gives `sysconf(_SC_NPROCESSORS_ONLN)`. A result below 1 tells that the system does not know, and gives 0.
macOS has no affinity of a process to processors, so `src/macos/program.c` gives this count.
`src/linux/program.c` gives it when `sched_getaffinity` fails.

## Functions

| Function | Result |
| --- | --- |
| `os_get_online_count()` | Gives the count of the processors that run, or 0. |

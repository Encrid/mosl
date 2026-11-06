# src/macos/program.c

On macOS, this file asks the system for the path of the program, and for the processors that run.

## The program

`_NSGetExecutablePath` gives the size that the path needs, then writes the path. That path can be relative, or hold a link.
`realpath` then makes it whole.

## The processors

macOS has no affinity of a process to processors. So `os_get_processor_count` gives `sysconf(_SC_NPROCESSORS_ONLN)`, the processors that run.
A result below 1 gives 0.

## Functions

| Function | Result |
| --- | --- |
| `os_get_program_path()` | Gives the whole path of the program, or `nullptr`. |
| `os_get_processor_count()` | Gives the count of the processors, or 0. |

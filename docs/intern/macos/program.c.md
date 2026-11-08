# src/macos/program.c

On macOS, this file asks the system for the path of the program, and for the processors that run.

## The program

`_NSGetExecutablePath` gives the size that the path needs, then writes the path. That path can be relative, or hold a link.
One `malloc` gives `PATH_MAX` bytes for the whole path, and after them the bytes of that first path.
`realpath` then makes the path whole at the start of the buffer. A path of a program keeps that limit on macOS.

## The processors

`os_get_processor_count` gives `os_get_online_count` of `src/posix/online.c`: `docs/intern/posix/online.c.md`, *The count*.

## Functions

| Function | Result |
| --- | --- |
| `os_get_program_path()` | Gives the whole path of the program, or `nullptr`. |
| `os_get_processor_count()` | Gives the result of `os_get_online_count`. |

# src/windows/program.c

On Windows, this file gives the path of the program, and `os_get_processor_count` from the masks of the process and of the system.

## The program

`os_get_program_path` asks `GetModuleFileNameW` for the path of the program. The buffer starts with 16 characters.
If the path fills it, the buffer doubles, and the call starts again. Thus each call runs the growth.
`os_narrow` turns the path into UTF-8.

## The processors

`GetProcessAffinityMask` gives the processors of the process, and the processors of the system, in one group of processors.
If the 2 masks differ, the process has a limit, and `__builtin_popcountg` counts the bits of its mask.
Else `GetActiveProcessorCount` with `ALL_PROCESSOR_GROUPS` counts the processors of each group.
A process with threads in more than one group gets 2 masks of 0, so it also gets the count of each group.

## Functions

| Function | Result |
| --- | --- |
| `os_get_program_path()` | Gives the path that `GetModuleFileNameW` writes, in UTF-8, or `nullptr`. |
| `os_get_processor_count()` | Gives the bits of the mask of the process, or the processors of each group, or 0. |

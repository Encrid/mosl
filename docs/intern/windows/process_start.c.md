# src/windows/process_start.c

This file starts a process on Windows.
Each function keeps the contract of `docs/extern/os.h.md`, *The processes*.

## The start

1. `os_prepare_launch` of `src/windows/process_program.c` makes the program, the command line, the environment block and the folder in UTF-16.
2. `open_parts` makes 4 events and the pipes: input, output, and errors if `errors_apart`: `src/windows/process_pipes.c`.
3. With `signals`, `open_parts` makes the pseudo console with `os_make_console` of `src/windows/process_console.c`.
4. `make_list` makes the attributes of the start: the ends that the child gets, and the pseudo console.
5. `run_process` calls `CreateProcessW` with `STARTF_USESTDHANDLES`, `CREATE_UNICODE_ENVIRONMENT`, and `CREATE_NEW_PROCESS_GROUP` for `ignore_interrupt`.
6. The start closes the ends of the child, and frees the texts. After a failure, `os_drop_process` frees each part that it made.

`PROC_THREAD_ATTRIBUTE_HANDLE_LIST` gives the child only its 3 ends, also though `CreateProcessW` lets it inherit handles.
The 2 names of the attributes are macros of the kind that `docs/intern/common/port_bytes.c.md`, *The order*, tells. So the file keeps their values in 2 constants.
Without `errors_apart`, the errors of the child go into the end of its output.
`open_parts` also gives the caller the action of Ctrl-C that `docs/extern/os.h.md`, *The processes*, tells.

## Limits

The ends of the child can be inherited. A start of another thread with no list of handles gives its child a copy of them for its life.
The result is the one of `docs/intern/posix/process_start.c.md`, *Limits*.

## Functions

| Function | Result |
| --- | --- |
| `make_list(handles, count, console)` | Gives the attributes of step 4, which the caller frees, or `nullptr`. |
| `free_list(list)` | Frees the attributes of step 4. A null `list` does nothing. |
| `open_parts(p, start, child)` | Does steps 2 and 3. Gives 0, or -1. |
| `run_process(p, start, launch)` | Does steps 4 and 5. Gives true if the process started. |
| `os_process_start(start)` | Starts the process as *The start* tells. Gives the process, or `nullptr`. |

# src/posix/process_pipes.c

This file makes the record of a process, writes into a process, and closes its pipes, on Linux and macOS.
It also closes the pipes of a start, and drops a process that did not start.
Each function keeps the contract of `docs/extern/os.h.md`, *The processes*.

## The record

`os_make_process` gives a record whose wake pipe and whose descriptor of the end are -1.
A descriptor of 0 is the standard input of the caller. Thus a record with zeros would make `os_drop_process` close it, after a start that fails before it opens a pipe.

## The write

`os_process_write` gives the input and the wake pipe to `os_write_all` of `src/posix/write_all.c`.
The input has `O_NONBLOCK`, so a full pipe gives `EAGAIN`, and `os_write_all` waits with `poll` for room or for the wake pipe.
A wake gives -1, as a closed input does.

## Functions

| Function | Result |
| --- | --- |
| `os_make_process()` | Gives a record with no descriptor, as *The record* tells, or `nullptr`. |
| `os_process_write(p, bytes, len)` | Writes the bytes as *The write* tells. Gives 0, or -1. |
| `os_process_close(p, stream)` | Closes the pipe `stream`, and sets its end to -1. |
| `os_close_pipes(pipes, count)` | Closes each end of the `count` pipes that is open, and sets it to -1. |
| `os_drop_process(p)` | Closes the wake pipe and the watch of the end, and frees the process. |

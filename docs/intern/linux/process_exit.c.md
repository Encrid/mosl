# src/linux/process_exit.c

This file gives the parts of a process that only Linux has: `pipe2`, and a descriptor of the process.

## The pipes

`os_open_pipe` calls `pipe2` with `O_CLOEXEC`, so no other child gets a copy of the ends.

## The end

`os_watch_exit` opens the descriptor with `os_pidfd_open` of `src/linux/pidfd.c`.
`os_wait_exit` waits with `poll` on that descriptor, which becomes ready at the end of the process.
A cut of `EINTR` is handled as in `docs/intern/posix/process_read.c.md`, *The read*.

## Functions

| Function | Result |
| --- | --- |
| `os_open_pipe(ends)` | Opens a pipe with `O_CLOEXEC`. Gives 0, or -1. |
| `os_watch_exit(p)` | Opens the descriptor of the process. Gives 0, or -1. |
| `os_wait_exit(p, deadline)` | Gives the result of `poll` on the descriptor: 0 at the end, 1 at the deadline, -1 on a failure. |

# src/linux/process_exit.c

This file gives the parts of a process that only Linux has: `pipe2`, and a descriptor of the process.

## The pipes

`os_open_pipe` calls `pipe2` with `O_CLOEXEC`, so no other child gets a copy of the ends.

## The end

`os_watch_exit` opens the descriptor with the system call `pidfd_open`. glibc 2.31 has no function for it, so the file calls `syscall`.
The targets of Linux need kernel 5.10 or newer, and `pidfd_open` came in kernel 5.3. So the call has no fallback.
`os_wait_exit` waits with `poll` on that descriptor, which becomes ready at the end of the process.

## Functions

| Function | Result |
| --- | --- |
| `os_open_pipe(ends)` | Opens a pipe with `O_CLOEXEC`. Gives 0, or -1. |
| `os_watch_exit(p)` | Opens the descriptor of the process. Gives 0, or -1. |
| `os_wait_exit(p, wait)` | Gives the result of `poll` on the descriptor: 0 at the end, 1 after `wait` milliseconds, -1 on a failure. |

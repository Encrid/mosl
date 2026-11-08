# src/macos/process_exit.c

On macOS, a pipe needs 2 calls, and a queue of events watches the end of a process. This file holds both.

## The pipes

macOS has no `pipe2`. So `os_open_pipe` calls `pipe`, then `fcntl` with `FD_CLOEXEC` on each end: `docs/intern/posix/process_start.c.md`, *Limits*.

## The end

`os_watch_exit` makes a queue with `kqueue`, and adds a filter `EVFILT_PROC` with `NOTE_EXIT` for the process.
`os_wait_exit` waits with `kevent` on the queue, which gives an event at the end of the process.

## Functions

| Function | Result |
| --- | --- |
| `os_open_pipe(ends)` | Opens a pipe, and sets `FD_CLOEXEC` on its 2 ends. Gives 0, or -1. |
| `os_watch_exit(p)` | Makes the queue and adds the filter. Gives 0, or -1. |
| `os_wait_exit(p, wait)` | Gives the result of `kevent`: 0 with the event of the end, 1 with no event after `wait` milliseconds, -1 on a failure. |

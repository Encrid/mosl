# src/macos/process_exit.c

On macOS, a pipe needs 2 calls, and a queue of events watches the end of a process. This file holds both.

## The pipes

macOS has no `pipe2`. So `os_open_pipe` calls `pipe`, then `fcntl` with `FD_CLOEXEC` on each end: `docs/intern/posix/process_start.c.md`, *Limits*.

macOS sends the `SIGPIPE` of a broken write to the process, and not to the thread that writes.
So a thread that does not block the signal takes it, and its default action ends the caller.
`os_quiet_pipe` sets `F_SETNOSIGPIPE` on the end of the input of the parent. A broken write then gives `EPIPE`, and no signal.
The child never holds that end, so its signals stay as `docs/extern/os.h.md`, *The processes*, tells.

## The end

`os_watch_exit` makes a queue with `kqueue`, and adds a filter `EVFILT_PROC` with `NOTE_EXIT` for the process.
The events have the type `os_kevent_t`: `docs/intern/macos/os_events.h.md`.
`os_wait_exit` waits with `kevent` on the queue, which gives an event at the end of the process.
With no deadline, `kevent` gets no time. `EINTR` makes it wait again: `docs/intern/posix/process_read.c.md`, *The read*.

## Functions

| Function | Result |
| --- | --- |
| `os_open_pipe(ends)` | Opens a pipe, and sets `FD_CLOEXEC` on its 2 ends. Gives 0, or -1. |
| `os_quiet_pipe(fd)` | Sets `F_SETNOSIGPIPE` on `fd`, as *The pipes* tells. |
| `os_watch_exit(p)` | Makes the queue and adds the filter. Gives 0, or -1. |
| `wait_once(p, deadline)` | Gives the result of one `kevent`: the count of events, or -1. |
| `os_wait_exit(p, deadline)` | Gives 0 with the event of the end, 1 with no event at the deadline, -1 on a failure. |

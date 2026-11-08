# src/posix/process_read.c

This file reads the output and the errors of a process on Linux and macOS, and gives the time of a wait.
Each function keeps the contract of `docs/extern/os.h.md`, *The processes*.

## The read

1. `poll` waits for the wake pipe, the output and the errors, up to the time that is left.
2. A byte in the wake pipe gives 0. `os_process_kill` wrote it: `docs/extern/os.h.md`, *The processes*.
3. `read` of a ready pipe takes at most 65536 bytes into the buffer of the process.
4. Bytes give `OS_OUTPUT` or `OS_ERRORS`. The end of a pipe closes it, and the read goes on with the other pipe.
5. When no pipe is open, the read gives 0. When the time passes, it gives -2.

The first pipe that the read asks changes at each read. Thus a pipe that always has bytes does not keep the other one waiting.
A signal that cuts `poll` gives `EINTR`, also with `SA_RESTART`. Then the read waits again for the time that is left.

## The time

`os_get_wait` gives `poll` and `kevent` the milliseconds until a deadline, rounded up: `docs/extern/os.h.md`, *The processes*.
`OS_FOREVER` gives -1, for no limit. A deadline that passed gives 0.
A wait of more than `INT_MAX` milliseconds, about 24 days, gives `INT_MAX`, and the wait goes on after it.

## Functions

| Function | Result |
| --- | --- |
| `os_get_wait(deadline)` | Gives the milliseconds of *The time*. |
| `take_bytes(p, fd, stream, len)` | Reads a ready pipe as steps 3 and 4 tell. Gives the result, or 1 to go on. |
| `read_once(p, deadline, len)` | Does one wait of *The read*. Gives the result, or 1 to go on. |
| `os_process_read(p, nanos, bytes, len)` | Reads as *The read* tells. |

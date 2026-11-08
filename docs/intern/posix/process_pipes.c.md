# src/posix/process_pipes.c

This file writes into a process, reads from it, and closes its pipes, on Linux and macOS.
Each function keeps the contract of `docs/extern/os.h.md`, *The processes*.

## The write

`os_process_write` gives the input and the wake pipe to `os_write_all` of `src/posix/write_all.c`.
The input has `O_NONBLOCK`, so a full pipe gives `EAGAIN`, and `os_write_all` waits with `poll` for room or for the wake pipe.
A wake gives -1, as a closed input does.

## The read

1. `poll` waits for the output, the errors and the wake pipe, up to the time that is left.
2. A byte in the wake pipe gives 0. `os_process_kill` wrote it: `docs/extern/os.h.md`, *The processes*.
3. `read` of a ready pipe takes at most 65536 bytes into the buffer of the process.
4. Bytes give `OS_OUTPUT` or `OS_ERRORS`. The end of a pipe closes it, and the read goes on with the other pipe.
5. When no pipe is open, the read gives 0. When the time passes, it gives -2.

The first pipe that the read asks changes at each read. Thus a pipe that always has bytes does not keep the other one waiting.
`get_wait` gives `poll` the time that is left, rounded up: `docs/extern/os.h.md`, *The processes*. `OS_FOREVER` gives no limit.

## Functions

| Function | Result |
| --- | --- |
| `os_process_write(p, bytes, len)` | Writes the bytes as *The write* tells. Gives 0, or -1. |
| `os_process_close(p, stream)` | Closes the pipe `stream`, if it is open. |
| `get_wait(deadline)` | Gives the milliseconds of `poll` until `deadline`, rounded up, or -1 for no limit. |
| `take_bytes(p, fd, stream, bytes, len)` | Reads a ready pipe as steps 3 and 4 tell. Gives the result, or 1 to go on. |
| `os_process_read(p, nanos, bytes, len)` | Reads as *The read* tells. |

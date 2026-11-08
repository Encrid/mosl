# src/posix/write_all.c

This file writes each byte into a descriptor, and keeps `SIGPIPE` from the process, on Linux and macOS.

## The signal

A write into a pipe with no reader sends `SIGPIPE` to the thread that writes, and the default action of `SIGPIPE` ends the process.
For such a write, the layer gives -1, and the process goes on: `docs/extern/os.h.md`, *The output*, and `docs/extern/os.h.md`, *The processes*.
So `os_write_all` blocks `SIGPIPE` in the thread for the time of the writes.

## The writes

1. `pthread_sigmask` blocks `SIGPIPE`, and keeps the old mask. `sigpending` tells if a `SIGPIPE` waits already.
2. `write` writes the bytes. After a short write or `EINTR`, it writes the rest again.
   After `EAGAIN`, `poll` waits for room in `fd` or for a byte of `wake`. A byte of `wake` gives -1. A `wake` of -1 is not asked.
3. If `write` failed with `EPIPE`, and no `SIGPIPE` waited before, `os_take_pipe_signal` takes the new `SIGPIPE`.
4. Linux and macOS differ when the program ignores `SIGPIPE`: `src/linux/pipe_signal.c` and `src/macos/pipe_signal.c`.
5. `pthread_sigmask` gives back the old mask.

A `SIGPIPE` that waited before the writes stays, so the process gets it as before.

## Functions

| Function | Result |
| --- | --- |
| `os_is_pipe_waiting()` | Gives true if a `SIGPIPE` waits for the thread. |
| `os_write_all(fd, wake, bytes, len)` | Writes the `len` bytes into `fd` with `SIGPIPE` blocked. Gives -1 at the first write that fails, else 0. |

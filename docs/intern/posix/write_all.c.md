# src/posix/write_all.c

This file writes each byte into a descriptor, and keeps `SIGPIPE` from the process, on Linux and macOS.

## The signal

A write into a pipe with no reader sends `SIGPIPE` to the thread that writes, and the default action of `SIGPIPE` ends the process.
For such a write, the layer gives -1, and the process goes on: `docs/extern/os.h.md`, *The output*, and `docs/extern/os.h.md`, *The processes*.
So `os_write_all` blocks `SIGPIPE` in the thread for the time of the writes.

## The writes

1. `pthread_sigmask` blocks `SIGPIPE`, and keeps the old mask. `sigpending` tells if a `SIGPIPE` waits already.
2. `write` writes the bytes. After a short write or `EINTR`, it writes the rest again.
3. If `write` failed with `EPIPE`, and no `SIGPIPE` waited before, `sigwait` takes the new `SIGPIPE`.
4. That `sigwait` does not wait, because the signal waits already.
5. `pthread_sigmask` gives back the old mask.

A `SIGPIPE` that waited before the writes stays, so the process gets it as before.

## Functions

| Function | Result |
| --- | --- |
| `os_write_all(fd, bytes, len)` | Writes the `len` bytes into `fd` with `SIGPIPE` blocked. Gives -1 at the first write that fails, else 0. |

# src/posix/process_pipes.c

This file writes into a process, and closes its pipes, on Linux and macOS.
Each function keeps the contract of `docs/extern/os.h.md`, *The processes*.

## The write

`os_process_write` gives the input and the wake pipe to `os_write_all` of `src/posix/write_all.c`.
The input has `O_NONBLOCK`, so a full pipe gives `EAGAIN`, and `os_write_all` waits with `poll` for room or for the wake pipe.
A wake gives -1, as a closed input does.

## Functions

| Function | Result |
| --- | --- |
| `os_process_write(p, bytes, len)` | Writes the bytes as *The write* tells. Gives 0, or -1. |
| `os_process_close(p, stream)` | Closes the pipe `stream`, and sets its end to -1. |

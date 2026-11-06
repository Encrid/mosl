# src/posix/output.c

On Linux and macOS, this file writes the bytes of `os_output_write` to `STDOUT_FILENO`.
Each function keeps the contract of `docs/extern/os.h.md`, *The output*.

## The write

`os_output_write` gives the bytes to `os_write_all` of `src/posix/write_all.c` with `STDOUT_FILENO`.
Thus a closed output gives -1, and does not stop the process with `SIGPIPE`.

## Functions

| Function | Result |
| --- | --- |
| `os_output_write(bytes, len)` | Gives the result of `os_write_all`. |

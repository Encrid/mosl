# src/posix/file_write.c

This file writes a file on Linux and macOS.
Each function keeps the contract of `docs/extern/os.h.md`, *The files*.

## The write

1. `os_open_parent` of `src/posix/parent.c` gives the folder of the last name, also for a path past `PATH_MAX`.
2. `openat` makes the file with `O_CREAT`, `O_EXCL` and the mode 0777 or 0666. The system removes the bits of the umask.
3. If the file was there, `openat` opens it again with `O_TRUNC`, and `fstat` reads its mode.
4. `get_kept_mode` keeps the read bits and the write bits, and sets or clears the execute bits.
5. If that mode is different, `fchmod` gives it to the file.
6. `os_write_all` of `src/posix/write_all.c` writes the bytes, and `close` closes the file.

The result is 0 only if each step passes.
A link at `path` fails the first `openat` with `EEXIST`. The second `openat` follows it, so the write goes into the target of the link.

## Functions

| Function | Result |
| --- | --- |
| `get_kept_mode(mode, executable)` | Gives the mode of a file that was there, as *The write* tells. |
| `open_file(at, name, executable)` | Opens the file as *The write* tells, and gives its descriptor, or -1. |
| `os_file_write(path, bytes, len, executable)` | Writes the file as *The write* tells. Gives 0, or -1. |

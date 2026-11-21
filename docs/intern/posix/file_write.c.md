# src/posix/file_write.c

This file writes a file on Linux and macOS.
Each function keeps the contract of `docs/extern/os.h.md`, *The files*.

## The write

1. `os_file_write` refuses an unknown flag first: `docs/intern/common/os_common.h.md`, *Constants*.
2. `os_open_parent` of `src/posix/parent.c` gives the folder of the last name, also for a path past `PATH_MAX`.
3. `openat` makes the file with `O_CREAT`, `O_EXCL` and the mode 0777 or 0666. The system removes the bits of the umask.
4. If the file was there, `openat` opens it again with `O_TRUNC`, and `fstat` reads its mode.
5. `get_kept_mode` keeps the read bits and the write bits, and sets or clears the execute bits.
6. If that mode is different, `fchmod` gives it to the file.
7. `os_write_all` of `src/posix/write_all.c` writes the bytes.
8. With `OS_FILE_DURABLE`, `fsync` flushes the file.
9. `close` closes the file.

The result is 0 only if each step passes.
A link at `path` fails the first `openat` with `EEXIST`. The second `openat` follows it, so the write goes into the target of the link.

## Functions

| Function | Result |
| --- | --- |
| `get_kept_mode(mode, executable)` | Gives the mode of a file that was there, as *The write* tells. `executable` is true for `OS_FILE_EXECUTABLE`. |
| `open_file(at, name, executable)` | Opens the file as *The write* tells, and gives its descriptor, or -1. |
| `os_file_write(path, bytes, len, flags)` | Writes the file as *The write* tells. Gives 0, or -1. |

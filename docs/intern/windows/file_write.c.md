# src/windows/file_write.c

This file writes a file on Windows.
Each function keeps the contract of `docs/extern/os.h.md`, *The files*.

## The write

A flag that is not known stops the write before its path: `docs/intern/common/os_common.h.md`, *Constants*.
Each function gives its path to `os_get_whole` of `src/windows/folder_open.c`. Thus a path past 260 characters works as a short one.
`CreateFileW` with `CREATE_ALWAYS` makes the file, or removes each byte of a file that is there.
`os_write_all` of `src/windows/write_all.c` writes the bytes, and `CloseHandle` closes the file.
A file has no execute bit on Windows, so `OS_FILE_EXECUTABLE` changes nothing.

With `OS_FILE_DURABLE`, `FlushFileBuffers` flushes the file before the close. The handle has `GENERIC_WRITE`, which the flush needs.

## Functions

| Function | Result |
| --- | --- |
| `os_file_write(path, bytes, len, flags)` | Writes the file as *The write* tells. Gives 0, or -1. |

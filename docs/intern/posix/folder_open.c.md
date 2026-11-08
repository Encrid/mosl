# src/posix/folder_open.c

This file opens and closes a folder of `include/os.h` on Linux and macOS.
Each function keeps the contract of `docs/extern/os.h.md`, *The folders*.

## The path

`os_folder_open` first gives the path to `opendir`, which opens a path of at most `PATH_MAX` bytes at once.
If `opendir` fails with `ENAMETOOLONG`, `os_open_parent` of `src/posix/parent.c` opens the folder before the last name.
Then `openat` opens the last name with `O_RDONLY`, and `fdopendir` makes its stream.
Thus the walk needs the right to read the last folder, as `opendir` of a short path.

## Limits

If the walk, `openat` or `fdopendir` fails, `os_folder_open` closes each descriptor. `close` of -1 does nothing.

## Functions

| Function | Result |
| --- | --- |
| `os_folder_open(path)` | Makes the folder with `calloc`, and opens its stream as *The path* tells. Gives `nullptr` if no stream opens. |
| `os_folder_close(f)` | Closes the stream with `closedir`, and frees the bytes and the folder. A null `f` does nothing. |

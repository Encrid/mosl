# src/posix/folder_open.c

This file opens and closes a folder of `include/os.h` on Linux and macOS.
Each function keeps the contract of `docs/extern/os.h.md`, *The folders*.

## The path

`os_folder_open` first gives the path to `opendir`, which opens a path of at most `PATH_MAX` bytes at once.
If `opendir` fails with `ENAMETOOLONG`, `os_open_parent` opens the folder before the last name, one name at a time.
Then `openat` opens the last name with `O_RDONLY`, and `fdopendir` makes its stream.

## The parent

`os_open_parent` gives the folder of the last name of a path, and that name. `src/posix/file.c` also calls it.
With no `make`, a path shorter than `PATH_MAX` needs no walk: the folder is `AT_FDCWD`, and the name is the path.
With `make`, the walk makes each folder before it opens it, for `src/posix/folder.c`.
`os_close_parent` closes a folder that is not `AT_FDCWD`.

A longer path starts the walk at `/` if it starts with `/`, else at the current folder.
`strtok_r` cuts a copy of the path before the last name at each `/`. `step` opens each name from the folder before it.
`strtok_r` gives no empty name, so a path that holds `//` gives `step` no empty name. The last name keeps a `/` at its end.

`step` opens each folder of the walk with `os_open_search`, for search only.
Thus the walk needs the same rights as `opendir` of a short path. It needs the right to search each folder before the last, and the right to read the last.

## Limits

If the walk, `openat` or `fdopendir` fails, `os_folder_open` closes each descriptor. `close` of -1 does nothing.

## Functions

| Function | Result |
| --- | --- |
| `step(at, name, make)` | Makes `name` in `at` if `make`, and opens it for search. Closes `at`, and gives the new descriptor, or -1. |
| `os_open_parent(path, name, make)` | Opens the folder before the last name, as *The parent* tells, or gives -1. Sets `*name`. |
| `os_close_parent(at)` | Closes `at` if it is not `AT_FDCWD`. |
| `os_folder_open(path)` | Makes the folder with `calloc`, and opens its stream as *The path* tells. Gives `nullptr` if no stream opens. |
| `os_folder_close(f)` | Closes the stream with `closedir`, and frees the bytes and the folder. A null `f` does nothing. |

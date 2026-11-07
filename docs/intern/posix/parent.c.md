# src/posix/parent.c

This file opens the folder of the last name of a path, also past `PATH_MAX`, on Linux and macOS.

## The parent

`os_open_parent` gives the folder of the last name of a path, and that name.
`src/posix/folder_open.c`, `src/posix/folder.c`, `src/posix/file_write.c`, `src/posix/entry.c` and `src/posix/remove.c` call it.
With no `make`, a path shorter than `PATH_MAX` needs no walk: the folder is `AT_FDCWD`, and the name is the path.
With `make`, the walk makes each folder before it opens it, for `src/posix/folder.c`.
`os_close_parent` closes a folder that is not `AT_FDCWD`.

A longer path starts the walk at `/` if it starts with `/`, else at the current folder.
A longer path that holds only the root opens `/` itself, with the right to read it, and names it `.`.
`strtok_r` cuts a copy of the path before the last name, which `malloc` gives, at each `/`. `step` opens each name from the folder before it.
`strtok_r` gives no empty name, so a path that holds `//` gives `step` no empty name. The last name keeps a `/` at its end.

`step` opens each folder of the walk with `os_open_search`, for search only.
Thus the walk needs the same rights as a call on a short path: the right to search each folder before the last.

## Functions

| Function | Result |
| --- | --- |
| `step(at, name, make)` | Makes `name` in `at` if `make`, and opens it for search. Closes `at`, and gives the new descriptor, or -1. |
| `get_last_start(path)` | Gives the place where the last name of `path` starts, after the `/` at the end. |
| `copy_start(path, len)` | Gives a new copy of the first `len` bytes of `path`. The caller frees it. `nullptr` tells that `malloc` failed. |
| `os_open_parent(path, name, make)` | Opens the folder before the last name, as *The parent* tells, or gives -1. Sets `*name`. |
| `os_close_parent(at)` | Closes `at` if it is not `AT_FDCWD`. |

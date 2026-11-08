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

A walk with no `make` opens each folder with `os_open_search`, for search only: `docs/intern/macos/folder_search.c.md`.
Thus a read needs the same rights as a call on a short path.

A walk with `make` opens each folder with the right to read it instead.
A make through a folder open for search only can land where the path shows no entry, on an overlay file system.
The right to read keeps the make on the path.

## Functions

| Function | Result |
| --- | --- |
| `step(at, name, make)` | Makes `name` in `at` if `make`, and opens it: readable with `make`, for search else. Closes `at`, and gives the descriptor, or -1. |
| `get_last_start(path)` | Gives the place where the last name of `path` starts, after the `/` at the end. |
| `copy_start(path, len)` | Gives a new copy of the first `len` bytes of `path`. The caller frees it. `nullptr` tells that `malloc` failed. |
| `os_open_parent(path, name, make)` | Opens the folder before the last name, as *The parent* tells, or gives -1. Sets `*name`. |
| `os_close_parent(at)` | Closes `at` if it is not `AT_FDCWD`. |

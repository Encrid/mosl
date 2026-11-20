# src/posix/entry.c

This file tells what is at a path, moves an entry and makes a link, on Linux and macOS.
Each function keeps the contract of `docs/extern/os.h.md`, *The files*.

Each function gives its path to `os_open_parent` of `src/posix/parent.c` with no `make`. It gives the folder of the last name, and that name.
Then `fstatat`, `renameat` or `symlinkat` works on the entry. Thus a path past `PATH_MAX` works as a short one.

## The entry

`fstatat` with no flag follows a link. `ENOENT` and `ENOTDIR` give 0, because nothing is at the path.
A folder gives 3. Another entry gives 2 if one of its 3 execute bits is set, else 1.

## The move

With a flag that is not known, the move opens no folder: `docs/intern/common/os_common.h.md`, *Constants*.
`renameat` moves the name of the folder of `from` to the name of the folder of `to`. It replaces a file at `to`.
Between 2 volumes, `renameat` fails with `EXDEV`, and the move gives -1.

With `OS_MOVE_DURABLE`, `os_sync_folder` of `src/posix/folder_sync.c` syncs the folder of `to` after `renameat`.
If `is_same_folder` tells that the text before the last name of `from` is different, it also syncs the folder of `from`.
Two texts can name one folder, for example `a/x` and `./a/y`. Then the call syncs that folder 2 times, and the result stays the same.
A failed sync gives -1, and the entry has its new name.

## Functions

| Function | Result |
| --- | --- |
| `os_path_entry(path)` | Tells what is at `path`, as *The entry* tells. |
| `is_same_folder(a, b)` | Tells if the texts of `a` and `b` before their last names are the same. |
| `os_path_move(from, to, flags)` | Moves the entry as *The move* tells. Gives 0, or -1. |
| `os_link_make(path, target)` | Makes the link with `symlinkat`. The system starts a relative `target` in the folder of the link. Gives 0, or -1. |

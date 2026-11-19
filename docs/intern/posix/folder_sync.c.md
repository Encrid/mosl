# src/posix/folder_sync.c

This file writes the entries of a folder to the disk, on Linux and macOS.
`os_path_move` of `src/posix/entry.c` calls it for a durable move: `docs/extern/os.h.md`, *The files*.

## The sync

`os_sync_folder` gets the folder `at` and the name `name` that `os_open_parent` of `src/posix/parent.c` gave.

1. `os_get_last_start` of `src/posix/parent.c` finds the start of the last name of `name`.
2. The text before the last name is the folder. With no text, the folder is `.` of `at`.
3. `openat` opens that folder from `at`, with the right to read it.
4. `fsync` writes the entries of the folder to the disk, and `close` closes the folder.

A path shorter than `PATH_MAX` gives `AT_FDCWD` and the whole path. Thus the folder of step 2 is shorter than `PATH_MAX` too, and fits on the stack.
A longer path gives the folder of the walk and the last name. That folder is open for search only, and `fsync` refuses such a descriptor on Linux. Thus step 3 opens `.` of it again.
A folder with no right to read fails step 3, and the call gives -1.

## Functions

| Function | Result |
| --- | --- |
| `os_sync_folder(at, name)` | Syncs the folder of `name`, as *The sync* tells. Gives 0, or -1. |

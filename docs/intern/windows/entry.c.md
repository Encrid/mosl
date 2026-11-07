# src/windows/entry.c

On Windows, `os_path_entry` reads the attributes of the target of a path, and `os_path_move` moves an entry.
Each function keeps the contract of `docs/extern/os.h.md`, *The files*.

Both calls take the whole paths of `os_get_whole`, also past 260 characters: `docs/intern/windows/folder_open.c.md`, *The path*.

## The entry

`CreateFileW` with `FILE_FLAG_BACKUP_SEMANTICS` opens a file or a folder, and follows a link.
`GetFileInformationByHandle` then gives its attributes: a folder gives 3, and a file gives 1.
`ERROR_FILE_NOT_FOUND` and `ERROR_PATH_NOT_FOUND` give 0, also for a link with no target.

## The move

`MoveFileExW` with `MOVEFILE_REPLACE_EXISTING` moves the entry, and replaces a file at `to`.
With no `MOVEFILE_COPY_ALLOWED`, a move between 2 volumes fails.

## Functions

| Function | Result |
| --- | --- |
| `get_whole_entry(whole)` | Gives 0, 1, 3 or -1 for the whole path, as *The entry* tells. |
| `os_path_entry(path)` | Gives the result of `get_whole_entry`, or -1. |
| `os_path_move(from, to)` | Moves the entry as *The move* tells. Gives 0, or -1. |

# src/windows/entry.c

On Windows, `os_path_entry` reads the attributes of the target of a path, and `os_path_move` moves an entry.
Each function keeps the contract of `docs/extern/os.h.md`, *The files*.

Both calls take the whole paths of `os_get_whole`, also past 260 characters: `docs/intern/windows/folder_open.c.md`, *The path*.

## The entry

`os_get_whole_info` reads the information of the entry of a whole path. `os_path_entry` and `os_folder_entry_size` of `src/windows/folder_read.c` use it.

`CreateFileW` with `FILE_FLAG_BACKUP_SEMANTICS` opens a file or a folder, and follows a link.
`GetFileInformationByHandle` then gives its attributes and its size, and the call gives 1.
`ERROR_FILE_NOT_FOUND` and `ERROR_PATH_NOT_FOUND` give 0, also for a link with no target. Each other failure gives -1.
`os_path_entry` gives 3 for a folder and 1 for a file.

## The move

`MoveFileExW` with `MOVEFILE_REPLACE_EXISTING` moves the entry, and replaces a file at `to`.
With no `MOVEFILE_COPY_ALLOWED`, a move between 2 volumes fails.

## Functions

| Function | Result |
| --- | --- |
| `os_get_whole_info(whole, info)` | Fills `info` and gives 1, or gives 0 or -1 for the whole path, as *The entry* tells. |
| `os_path_entry(path)` | Gives 0, 1, 3 or -1, as *The entry* tells. |
| `os_path_move(from, to)` | Moves the entry as *The move* tells. Gives 0, or -1. |

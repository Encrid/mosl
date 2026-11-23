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

An unknown flag gives -1 before `os_get_whole`: `docs/intern/common/os_common.h.md`, *Constants*.

`rename_posix` moves the entry first. It opens `from` with `DELETE` access, `OS_SHARE_ALL`, `FILE_FLAG_BACKUP_SEMANTICS` for a folder, and `FILE_FLAG_OPEN_REPARSE_POINT`, so a link moves itself and not its target.
`SetFileInformationByHandle` with `FileRenameInfoEx` then gives it the whole path `to`, with `FILE_RENAME_FLAG_REPLACE_IF_EXISTS` and `FILE_RENAME_FLAG_POSIX_SEMANTICS`. Thus it replaces a file at `to` that another handle holds open with the share of the delete.

If `rename_posix` fails, `MoveFileExW` with `MOVEFILE_REPLACE_EXISTING` moves the entry, and replaces a file at `to`. Thus a file system with no rename of POSIX still moves.

With no `MOVEFILE_COPY_ALLOWED`, a move between 2 volumes fails, and the rename of `FileRenameInfoEx` stays on 1 volume too.
With `OS_MOVE_DURABLE`, the flags of `MoveFileExW` also hold `MOVEFILE_WRITE_THROUGH`. The rename of `rename_posix` has no such flag. `docs/extern/os.h.md`, *The files*, tells what this flag gives, and the limit of its guarantee.

## Functions

| Function | Result |
| --- | --- |
| `os_get_whole_info(whole, info)` | Fills `info` and gives 1, or gives 0 or -1 for the whole path, as *The entry* tells. |
| `os_path_entry(path)` | Gives 0, 1, 3 or -1, as *The entry* tells. |
| `rename_posix(from, to)` | Renames the whole path `from` to the whole path `to` with `FileRenameInfoEx`, as *The move* tells. Gives true on success. |
| `os_path_move(from, to, flags)` | Moves the entry as *The move* tells. Gives 0, or -1. |

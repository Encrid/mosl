# src/windows/file.c

On Windows, this file gives the calls on a file and on a path, with the whole paths of `os_get_whole`.
Each function keeps the contract of `docs/extern/os.h.md`, *The files*.

## The path

Each function gives its path to `os_get_whole` of `src/windows/folder_open.c`. Thus a path past 260 characters works as a short one.

## The write

`CreateFileW` with `CREATE_ALWAYS` makes the file, or removes each byte of a file that is there.
`os_write_all` of `src/windows/write_all.c` writes the bytes, and `CloseHandle` closes the file.
A file has no execute bit on Windows, so `executable` changes nothing.

## The entry

`CreateFileW` with `FILE_FLAG_BACKUP_SEMANTICS` opens a file or a folder, and follows a link.
`GetFileInformationByHandle` then gives its attributes: a folder gives 3, and a file gives 1.
`ERROR_FILE_NOT_FOUND` and `ERROR_PATH_NOT_FOUND` give 0, also for a link with no target.

## The move

`MoveFileExW` with `MOVEFILE_REPLACE_EXISTING` moves the entry, and replaces a file at `to`.
With no `MOVEFILE_COPY_ALLOWED`, a move between 2 volumes fails.

## The remove

`remove_entry` reads the attributes of the entry with `GetFileAttributesW`, which does not follow a link:

1. If the entry has `FILE_ATTRIBUTE_READONLY`, `SetFileAttributesW` removes it.
2. A folder that is not a link gets the search of `FindFirstFileExW`. `remove_entry` removes each of its entries, then `RemoveDirectoryW` removes it.
3. A link to a folder has `FILE_ATTRIBUTE_DIRECTORY` and `FILE_ATTRIBUTE_REPARSE_POINT`. `RemoveDirectoryW` removes the link, and keeps the target.
4. Each other entry goes with `DeleteFileW`.

`joined` makes the whole path of each entry of a folder, because the path of a deep entry can be long.

## The link

`os_link_make` changes each `/` of `target` into `\`, because Windows follows only `\` in a relative target.
It joins the folder of `path` and a relative `target`, and `GetFileAttributesW` tells if a folder is there.
If it is, `CreateSymbolicLinkW` gets `SYMBOLIC_LINK_FLAG_DIRECTORY`. It always gets `SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE`, which works in the Developer Mode.

## Functions

| Function | Result |
| --- | --- |
| `os_file_write(path, bytes, len, executable)` | Writes the file as *The write* tells. Gives 0, or -1. |
| `os_path_entry(path)` | Gives 0, 1 or 3 from the attributes of the target. |
| `os_path_move(from, to)` | Moves the entry as *The move* tells. Gives 0, or -1. |
| `joined(folder, name)` | Gives the whole path of `name` in `folder`, which the caller frees, or `nullptr`. |
| `remove_entry(whole)` | Removes the entry of the whole path, as *The remove* tells. Gives 0, or -1. |
| `os_path_remove(path)` | Gives 1 if `GetFileAttributesW` finds nothing, else the result of `remove_entry`. |
| `os_link_make(path, target)` | Makes the link as *The link* tells. Gives 0, or -1. |

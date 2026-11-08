# src/windows/remove.c

On Windows, `os_path_remove` takes away a file, a link, or a folder with its entries.
Each function keeps the contract of `docs/extern/os.h.md`, *The files*.

## The remove

`os_remove_entry` reads the attributes of the entry with `GetFileAttributesW`, which does not follow a link:

1. If the entry has `FILE_ATTRIBUTE_READONLY`, `SetFileAttributesW` removes it.
2. A folder that is not a link gets the search of `FindFirstFileExW` in `remove_entries`.
3. `remove_child` calls `os_remove_entry` for each entry of the folder, then `RemoveDirectoryW` removes the folder.
4. A link to a folder has `FILE_ATTRIBUTE_DIRECTORY` and `FILE_ATTRIBUTE_REPARSE_POINT`. `RemoveDirectoryW` removes the link, and keeps the target.
5. Each other entry goes with `DeleteFileW`.

`join_path` makes the whole path of each entry of a folder, because the path of a deep entry can be long.
`os_remove_entry` is in `src/windows/os_types.h`, since the 2 functions of the walk call each other.
`os_path_remove` gives 1 when `GetFileAttributesW` fails with `ERROR_FILE_NOT_FOUND` or `ERROR_PATH_NOT_FOUND`.

## Functions

| Function | Result |
| --- | --- |
| `join_path(folder, name)` | Gives the whole path of `name` in `folder`, which the caller frees, or `nullptr`. |
| `remove_child(folder, name)` | Gives the result of `os_remove_entry` for the entry `name` of `folder`, or -1. |
| `remove_entries(whole)` | Takes away each entry of the folder through `remove_child`. Gives 0, or -1. |
| `os_remove_entry(whole)` | Removes the entry of the whole path, as *The remove* tells. Gives 0, or -1. |
| `os_path_remove(path)` | Gives 1 when `GetFileAttributesW` finds nothing, or the result of `os_remove_entry`. |

# src/windows/folder.c

This file holds the folder calls for Windows.

`os_folder_set` turns the path into UTF-16 with `os_widen`, and gives it to `SetCurrentDirectoryW`. Thus it keeps the limit of 260 characters.

## The make

`os_folder_make` makes the whole path of the folder with `os_get_whole` of `src/windows/folder_open.c`:

1. `os_get_root_end` of `src/windows/whole_form.c` finds the end of the root of the path.
2. For each `\` after the root, `os_folder_make` ends the path there for a moment, and `CreateDirectoryW` makes that folder.
3. At the end, `CreateDirectoryW` makes the last folder.
4. `ERROR_ALREADY_EXISTS` is not an error if `GetFileAttributesW` finds a folder at the path.

## Functions

| Function | Result |
| --- | --- |
| `os_folder_set` | Enters the working folder with `SetCurrentDirectoryW`. |
| `make_one(whole)` | Makes the folder `whole`, or finds a folder there. Gives 0, or -1. |
| `os_folder_make` | Makes the folder and each folder above it, as *The make* tells. Gives 0, or -1. |

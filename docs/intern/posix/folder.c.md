# src/posix/folder.c

This file gives the folder calls for Linux and macOS.

`os_folder_set` calls `chdir`, so it keeps the limit of `PATH_MAX`.

## The make

`os_folder_make` gives the path to `os_open_parent` of `src/posix/parent.c` with `make`.
The walk then makes each folder before the last name with `mkdirat`, before it opens it, also past `PATH_MAX`.
At the end, `mkdirat` makes the last folder with the mode 0777, less the umask.
A folder that is there already is not an error: `EEXIST` gives 0 when the name opens for search. A folder opens, and a file does not.

## Functions

| Function | Result |
| --- | --- |
| `os_folder_set` | Changes the working folder through `chdir`. |
| `os_folder_make` | Gives the result of the last `mkdirat`, or 0 for a folder that is there. |

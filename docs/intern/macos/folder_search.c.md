# src/macos/folder_search.c

On macOS, this file gives the walk of `src/posix/parent.c` a folder that is open for search only.
Each function keeps the contract of `docs/extern/os.h.md`, *The folders*.

## The flag

`os_open_search` opens the folder with `O_SEARCH`, which needs only the right to search the folder.
macOS 13, the oldest system of the targets, has the flag: its kernel defines `O_EXEC` and `O_SEARCH`, and that of macOS 12 does not.
`O_SEARCH` holds `O_DIRECTORY`, so the call refuses a file that is not a folder.

## Functions

| Function | Result |
| --- | --- |
| `os_open_search(at, name)` | Opens `name` in `at` with `O_SEARCH` and `O_CLOEXEC`. Gives the new descriptor, or -1 if `openat` fails. |

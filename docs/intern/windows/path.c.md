# src/windows/path.c

On Windows this file turns a path into its whole form.
`os_whole_text` turns the path of UTF-8 into UTF-16 with `os_widen`, and `GetFullPathNameW` makes it whole.
`os_path_whole` gives its result, and `src/windows/env.c` calls it.
`os_narrow` turns the whole path back into UTF-8. `_fullpath` would read the path in the code page of the system, and not in UTF-8.

## Functions

| Function | Result |
| --- | --- |
| `os_whole_text` | Gives the whole path from `GetFullPathNameW`, or null. |
| `os_path_whole` | Gives the result of `os_whole_text`. |

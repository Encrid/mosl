# src/posix/base_join.c

`os_join_base` joins a folder and a rest into a base folder of the user, for `os_folder_base` on Linux and macOS.

## The join

`base` must be an absolute path: it starts with `/`.
A `base` that is `nullptr`, empty or relative gives `nullptr`, and takes no memory.
Else the result is `base`, then `rest`, in memory that the caller frees.

## Functions

| Function | Result |
| --- | --- |
| `os_join_base(base, rest)` | The join of *The join*, or `nullptr`. |

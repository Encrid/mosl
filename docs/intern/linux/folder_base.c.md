# src/linux/folder_base.c

On Linux, `os_folder_base` reads the base folder of XDG.
It keeps the contract of `docs/extern/os.h.md`, *The base folders*.

## The base

1. `os_is_folder_kind` of `src/common/os_common.h` refuses a kind that is not known.
2. If `XDG_DATA_HOME` starts with `/`, it is the base.
3. Else the base is `HOME` and `/.local/share`, by `os_join_base` of `src/posix/base_join.c`.

The 2 kinds give the same base.

## Functions

| Function | Result |
| --- | --- |
| `os_folder_base(kind)` | The base of *The base*, or `nullptr`. |

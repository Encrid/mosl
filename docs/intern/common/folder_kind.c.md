# src/common/folder_kind.c

This file holds the check of the kind of a base folder, which each system shares.
Each function keeps the contract of `docs/extern/os.h.md`, *The base folders*.

## The kind

`os_folder_base` of each system calls `os_is_folder_kind` first, and gives `nullptr` for a kind that it refuses.

## Functions

| Function | Result |
| --- | --- |
| `os_is_folder_kind(kind)` | Tells if `kind` is `OS_FOLDER_USER` or `OS_FOLDER_LOCAL`. |

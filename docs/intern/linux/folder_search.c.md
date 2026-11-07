# src/linux/folder_search.c

This file opens a folder of a long path on Linux, for the walk of `src/posix/parent.c`.
Each function keeps the contract of `docs/extern/os.h.md`, *The folders*.

## The flag

`os_open_search` opens the folder with `O_PATH`. Linux then checks no right on the folder itself, only the right to search the folder before it.
The descriptor serves only as the folder of the next `openat`. Thus the walk needs the right to search each folder, as a short path does.
glibc declares `O_PATH` with `_GNU_SOURCE`, which `system_defines` of `.toolchain/config/targets.toml` gives each Linux target.

## Functions

| Function | Result |
| --- | --- |
| `os_open_search(at, name)` | Calls `openat` with `O_PATH`, `O_DIRECTORY` and `O_CLOEXEC`, and gives what it gives: a descriptor, or -1. |

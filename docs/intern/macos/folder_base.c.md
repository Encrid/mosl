# src/macos/folder_base.c

On macOS, `os_folder_base` gives the folder Application Support of the user.
It keeps the contract of `docs/extern/os.h.md`, *The base folders*.

After the check of the kind, each known kind gives `HOME` and `/Library/Application Support`, by `os_join_base` of `src/posix/base_join.c`.

## Functions

| Function | Result |
| --- | --- |
| `os_folder_base(kind)` | The folder Application Support, or `nullptr`. |

# src/windows/link.c

On Windows, this file makes a symbolic link.
Each function keeps the contract of `docs/extern/os.h.md`, *The files*.

## The link

`os_link_make` changes each `/` of `target` into `\`, because Windows follows only `\` in a relative target.
`get_target_kind` joins the folder of `path` and a relative `target`, and `GetFileAttributesW` tells if a folder is there.
`is_absolute` tells a target that starts with a separator or with a drive, which needs no folder before it.
If the memory of that path fails, the link is not made, because its kind is not known.
A link to a folder gets `SYMBOLIC_LINK_FLAG_DIRECTORY`. Each link gets `SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE`, which works in the Developer Mode.

## Functions

| Function | Result |
| --- | --- |
| `is_absolute(target)` | Gives true if `target` starts with `/`, with `\`, or with a drive. |
| `get_target_kind(path, target)` | Gives 1 if `target` names a folder from the folder of `path`, 0 if not, or -1 if its memory fails. |
| `os_link_make(path, target)` | Makes the link as *The link* tells. Gives 0, or -1. |

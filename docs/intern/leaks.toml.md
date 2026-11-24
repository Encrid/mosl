# src/leaks.toml

This file names each resource that a user of the layer takes from it, for the count of `test --leaks` of the user.
A release ships the file as `leaks.toml`: `docs/extern/os.h.md`.

- `memory` names the functions that give memory which the caller frees: `os_path_whole`, `os_env_get`, `os_path_find`, `os_get_program_path` and `os_folder_base`.
- `pairs` names each function that takes a resource, with the function that gives it back.
- The kind `os_lock` names `os_lock`, which writes the handle of a lock through its argument 2, and `os_unlock`, which gives it back.

## The count of the layer itself

The layer does not count with this file. A user sees only the call of a function. The count of the layer also sees the inside of the function.
If the inside takes the memory with `malloc`, the count has the block already. The same function in `memory` would count it a second time, at each call of a test.

Thus `.toolchain/config/leaks.toml` is the list of the layer. It names the same pairs, and it reads `tests/leaks/<system>.toml`:

| File | `memory` | Reason |
| --- | --- | --- |
| `tests/leaks/windows.toml` | No function. | On Windows, each of the 5 functions takes its memory with `malloc`. |
| `tests/leaks/posix.toml` | `os_path_find`, `os_path_whole`. | On Linux and on macOS, these 2 get their memory from `realpath`, which the count does not see. `os_env_get` takes it with `strdup`, and `os_get_program_path` and `os_folder_base` take it with `malloc`. |

A change of the inside of a function of `memory` can change how it takes its memory. Its place in these 2 files then changes too.

# src/windows/env.c

On Windows, this file gives `os_env_get`, `os_path_find` and `os_get_program_path`.
Each function keeps the contract of `docs/extern/os.h.md`, *The environment*.

## The variable

`os_env_get` turns the name into UTF-16 with `os_widen`, and asks `GetEnvironmentVariableW` for the size of the value.
`ERROR_ENVVAR_NOT_FOUND` gives `nullptr`. Else the second call writes the value, and `os_narrow` turns it into UTF-8.

## The search

The search follows `docs/intern/posix/env.c.md`, *The search*, with these differences:

- A `\` also makes a name a path. `PATH` has a `;` between 2 folders, and an empty item names no folder.
- A name with no `.` gets the extensions of `PATHEXT`: `docs/extern/os.h.md`, *The environment*.
- `is_program` asks `GetFileAttributesW` for an entry that is not a folder.
- `os_path_whole` of `src/windows/path.c` makes the path whole.

With no `PATHEXT` in the environment, the call tries no extension.

## The program

`os_get_program_path` asks `GetModuleFileNameW` for the path of the program. The buffer starts with 260 characters.
If the path fills it, the buffer doubles, and the call starts again. `os_narrow` turns the path into UTF-8.

## Functions

| Function | Result |
| --- | --- |
| `is_program(path)` | Gives true if an entry that is not a folder is at `path`. |
| `os_env_get(name)` | Gives the value in UTF-8, or `nullptr`. |
| `os_path_find(name)` | Gives the first path that `is_program` passes, or `nullptr`. |
| `os_get_program_path()` | Gives the path that `GetModuleFileNameW` writes, in UTF-8, or `nullptr`. |

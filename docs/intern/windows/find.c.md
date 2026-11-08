# src/windows/find.c

On Windows, this file finds a program on the `PATH`.
Each function keeps the contract of `docs/extern/os.h.md`, *The environment*.

## The search

The search follows `docs/intern/posix/env.c.md`, *The search*, with these differences:

- A `\` also makes a name a path. `PATH` has a `;` between 2 folders, and an empty item names no folder.
- A name with no `.` gets the extensions of `PATHEXT`: `docs/extern/os.h.md`, *The environment*.
- `is_program` asks `GetFileAttributesW` for an entry that is not a folder.
- `os_whole_text` of `src/windows/path.c` makes the path whole, and `os_get_variable` of `src/windows/env.c` reads `PATH` and `PATHEXT`.

With no `PATHEXT` in the environment, the call tries no extension.

## Functions

| Function | Result |
| --- | --- |
| `is_program(path)` | Gives true if an entry that is not a folder is at `path`. |
| `find_with(path, at, exts)` | Tries the name at `at` of `path` with each extension of `exts`, then alone. Gives the whole path of the first program, or `nullptr`. |
| `find_in(folders, name, exts)` | Tries each folder of `folders` with `find_with`. Gives the first program, or `nullptr`. |
| `os_find_text(name)` | Gives the first path that `is_program` passes, or `nullptr`. `src/windows/process_program.c` calls it, so `test --leaks` counts its memory one time. |
| `os_path_find(name)` | Gives the result of `os_find_text`. |

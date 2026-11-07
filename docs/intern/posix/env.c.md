# src/posix/env.c

This file reads a variable of the environment, and finds a program on the `PATH`, on Linux and macOS.
Each function keeps the contract of `docs/extern/os.h.md`, *The environment*.

## The variable

`os_env_get` copies the value that `getenv` gives with `strdup`. The layer never changes the environment.
A thread of the caller that changes the environment during the call makes a race, as in each program of C.

## The search

1. A name with a `/` is a path. `is_program` checks it, and `realpath` makes it whole.
2. Else `os_path_find` cuts a copy of `PATH` at each `:`. An empty item names the current folder.
3. For each folder, it joins the folder, a `/` and the name.
4. `is_program` checks the joined path: `stat` must find a regular file with one of its 3 execute bits.
5. The first path that passes gives the result, made whole by `realpath`.

With no `PATH` in the environment, the call finds nothing, and gives `nullptr`.

## Functions

| Function | Result |
| --- | --- |
| `is_program(path)` | Gives true if `path` is a regular file with an execute bit. |
| `os_env_get(name)` | Gives a copy of the value of `name`, or `nullptr`. |
| `os_path_find(name)` | Gives the whole path of the program, as *The search* tells, or `nullptr`. |

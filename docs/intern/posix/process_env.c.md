# src/posix/process_env.c

This file finds the program of a process, and makes its environment, on Linux and macOS.

## The program

`os_find_program` copies `argv[0]` if it holds a `/`. A relative path then starts in `folder`, because the child goes there before `execve`.
Else `os_path_find` finds the program on the `PATH` of the caller.
The copy comes from `malloc`, so `test --leaks` counts it with the `free` of the start.

## The environment

`os_build_env` gives one array of pointers, with `nullptr` after the last.
It keeps each variable of `environ` whose name is not in `unset` and not in `env`. Then it adds each text of `env`.
The array points into `environ` and into `env`, and copies no text. The parent frees it after `fork`.

## Functions

| Function | Result |
| --- | --- |
| `has_name(entry, names)` | Gives true if the name before the `=` of `entry` is a name of `names`. A text of `names` can hold its own `=`. |
| `count_texts(texts)` | Gives the count of the texts before `nullptr`, or 0 for a null array. |
| `os_build_env(env, unset)` | Gives the environment of *The environment*, which the caller frees, or `nullptr`. |
| `os_find_program(name)` | Gives the path of *The program*, which the caller frees, or `nullptr`. |

# src/posix/process_env.c

This file makes the environment of a process on Linux and macOS.

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

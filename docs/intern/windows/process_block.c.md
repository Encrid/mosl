# src/windows/process_block.c

On Windows, this file makes the environment block of a process.

## The block

The block holds each variable as `NAME=value` in UTF-16, with a 0 after each, and one more 0 at the end.
Its variables come from `GetEnvironmentStringsW`, `env` and `unset`, by the rule of `docs/intern/posix/process_env.c.md`, *The environment*.
`CompareStringOrdinal` compares 2 names with no case: `docs/extern/os.h.md`, *The processes*.

A name can start with `=`, as the folders of the drives do, for example `=C:`. So the name ends at the first `=` after its first character.
`copy_block` runs 2 times: first with no block, to count the characters, then to copy them.

## Functions

| Function | Result |
| --- | --- |
| `get_name_len(entry)` | Gives the count of the characters of the name of `entry`. |
| `has_name(entry, names)` | Gives true if the name of `entry` is a name of `names`. |
| `copy_block(out, system, env, unset)` | Copies the variables of *The block* into `out`, if `out` is not null, and gives their count of characters. |
| `os_build_block(env, unset)` | Gives the block, which the caller frees, or `nullptr`. |

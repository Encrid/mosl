# src/windows/process_program.c

This file turns the program, the arguments, the environment and the folder of a start into the texts of `CreateProcessW`, on Windows.

## The program

A name with no `\\` and no `/` goes to `os_find_text` of `src/windows/find.c`, which finds it on the `PATH` with `PATHEXT`.
Else the name is a path. `join_folder` puts `folder` before a relative path, because `CreateProcessW` starts a relative path in the folder of the caller.
A whole path starts with a separator, or has a `:` after its first character, as a letter of a drive does. Then `os_whole_text` makes the path whole, and `os_widen` turns it into UTF-16.

## The launch

`os_prepare_launch` fills `os_launch_t`: the program, the command line of `os_join_line`, the environment block of `os_build_block`, and the folder in UTF-16.
It gives false if one of them fails.

## Functions

| Function | Result |
| --- | --- |
| `join_folder(start)` | Gives the path of the program, from `folder` if it is relative, which the caller frees, or `nullptr`. |
| `os_find_program(start)` | Gives the whole path of the program in UTF-16, as *The program* tells, or `nullptr`. |
| `os_prepare_launch(start, launch)` | Fills the texts of the launch, as *The launch* tells. Gives true, or false. |

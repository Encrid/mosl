# src/windows/whole_form.c

This file gives a whole path of Windows the form that takes a long path.
Each function keeps the contract of `docs/extern/os.h.md`, *The folders*.

## The form

The whole path starts `OS_PLACE` characters after the start of the buffer, so the form can grow in place.
`os_mark_whole` puts `\\?\` before a path that starts with a letter of a drive.
It puts `\\?\UNC` in place of the first backslash of a network path, `\\server\share`.
A path that starts with `\\?\` or `\\.\` keeps its form, and moves to the start of the buffer.
With the form `\\?\`, Windows gives the path to the file system with no change, so the path can be long.

Then `os_mark_whole` drops one last backslash, so that a joined name follows one backslash. Windows does not fold two of them in that form.
Thus `C:\mods\` gives `\\?\C:\mods`, and the root `C:\` gives `\\?\C:`.
`GetFullPathNameW` keeps one last backslash of a path, and folds a run of them into one. So one backslash at most is left to drop.

## The root

`os_get_root_end` gives the place of the 4th `\` of a whole path, or of the 6th for the form `\\?\UNC\`.
A path with fewer gives its end.
That place ends the root: `\\?\C:`, the share of `\\?\UNC\server\share`, or the device of `\\.\device`.
`src/windows/folder.c` makes no folder of the root.

## The test

The file uses only the C of the standard. Thus the program of `tests/whole_test.c` compiles it on each system, and the cases check each form.

## Functions

| Function | Result |
| --- | --- |
| `os_mark_whole(whole, size)` | Gives the whole path in `whole` the form of *The form*. `size` counts its characters and its end. |
| `os_get_root_end(whole)` | Gives the place where the root of `whole` ends, as *The root* tells. |

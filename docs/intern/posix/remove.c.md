# src/posix/remove.c

This file removes what is at a path, with each entry of a folder, on Linux and macOS.
Each function keeps the contract of `docs/extern/os.h.md`, *The files*.

## The remove

`os_remove_entry` removes one entry `name` of the folder `at`:

1. `fstatat` with `AT_SYMLINK_NOFOLLOW` reads the entry. A failure gives -1.
2. An entry that is not a folder goes with `unlinkat` and no flag.
3. For a folder, `open_writable` gives the owner each right with `fchmodat`, if the folder lacks one.
4. `openat` and `fdopendir` open the folder. `remove_entries` calls `os_remove_entry` for each of its entries.
5. `unlinkat` with `AT_REMOVEDIR` removes the empty folder.
6. The first failure stops the walk, and gives -1.

- `fstatat` with `AT_SYMLINK_NOFOLLOW` does not follow a link. So `unlinkat` removes the link, and keeps its target.
- The write bit of a folder lets `unlinkat` remove its entries. So the owner gets each right on a folder before its read, also on a read-only folder.
- A file that is read-only goes too, because `unlinkat` needs the right to write the folder, and not the file.
- `os_path_remove` gives 1 when `os_open_parent` or the first `fstatat` fails with `ENOENT` or `ENOTDIR`.

`src/posix/os_types.h` declares `os_remove_entry`, because it and `remove_entries` call each other.

## Limits

Each level of the walk keeps 1 folder open. Thus the depth of a tree is limited by the count of descriptors that the process can open.

## Functions

| Function | Result |
| --- | --- |
| `open_writable(at, name, mode)` | Gives the owner each right on the folder, and opens its stream, or gives `nullptr`. |
| `remove_entries(at, name, mode)` | Removes each entry of the folder, as *The remove* tells. Gives 0, or -1. |
| `os_remove_entry(at, name)` | Removes the entry `name` of the folder `at`, as *The remove* tells. Gives 0, or -1. |
| `os_path_remove(path)` | Gives 1 if nothing is at `path`, else the result of `os_remove_entry`. |

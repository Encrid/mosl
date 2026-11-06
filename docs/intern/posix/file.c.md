# src/posix/file.c

This file writes, tells, moves, removes and links the entries of a path for Linux and macOS. A long path goes through `os_open_parent`.
Each function keeps the contract of `docs/extern/os.h.md`, *The files*.

## The path

Each function gives its path to `os_open_parent` of `src/posix/folder_open.c` with no `make`. It gives the folder of the last name, and that name.
Then a call with the folder and the name works on the entry: `openat`, `fstatat`, `renameat`, `unlinkat` or `symlinkat`.
Thus a path past `PATH_MAX` works as a short one. `os_close_parent` closes the folder at the end.

## The write

1. `openat` makes the file with `O_CREAT`, `O_EXCL` and the mode 0777 or 0666. The system removes the bits of the umask.
2. If the file was there, `openat` opens it again with `O_TRUNC`, and `fstat` reads its mode.
3. `kept_mode` keeps the read bits and the write bits, and sets or clears the execute bits: `docs/extern/os.h.md`, *The files*.
4. If that mode is different, `fchmod` gives it to the file.
5. `os_write_all` of `src/posix/write_all.c` writes the bytes.
6. `close` closes the file. The result is 0 only if each step passes.

A link at `path` fails the first `openat` with `EEXIST`. The second `openat` follows it, so the write goes into the target of the link.

## The entry

`fstatat` with no flag follows a link. `ENOENT` and `ENOTDIR` give 0, because nothing is at the path.
A folder gives 3. Another entry gives 2 if one of its 3 execute bits is set, else 1.

## The move

`renameat` moves the name of the folder of `from` to the name of the folder of `to`. It replaces a file at `to`.
Between 2 volumes, `renameat` fails with `EXDEV`, and the move gives -1.

## The remove

`remove_entry` removes one entry `name` of the folder `at`:

1. `fstatat` with `AT_SYMLINK_NOFOLLOW` reads the entry. A failure gives -1.
2. An entry that is not a folder goes with `unlinkat` and no flag.
3. For a folder, `fchmodat` gives the owner each right, if the folder lacks one.
4. `openat` and `fdopendir` open the folder. `remove_entry` removes each of its entries.
5. `unlinkat` with `AT_REMOVEDIR` removes the empty folder.
6. The first failure stops the walk, and gives -1.

- `fstatat` with `AT_SYMLINK_NOFOLLOW` does not follow a link. So `unlinkat` removes the link, and keeps its target.
- The write bit of a folder lets `unlinkat` remove its entries. So `fchmodat` gives the owner each right on a folder before its read, also when the folder is read-only.
- A file that is read-only goes too, because `unlinkat` needs the right to write the folder, and not the file.
- `os_path_remove` gives 1 when the first `fstatat` fails with `ENOENT`.

## Limits

Each level of `remove_entry` keeps 1 folder open. Thus the depth of a tree is limited by the count of descriptors that the process can open.

## Functions

| Function | Result |
| --- | --- |
| `kept_mode(mode, executable)` | Gives the mode of a file that was there, as *The write* tells. |
| `os_file_write(path, bytes, len, executable)` | Writes the file as *The write* tells. Gives 0, or -1. |
| `os_path_entry(path)` | Tells what is at `path`, as *The entry* tells. |
| `os_path_move(from, to)` | Moves the entry as *The move* tells. Gives 0, or -1. |
| `remove_entry(at, name)` | Removes the entry `name` of the folder `at`, as *The remove* tells. Gives 0, or -1. |
| `os_path_remove(path)` | Gives 1 if the first `fstatat` finds nothing, else the result of `remove_entry`. |
| `os_link_make(path, target)` | Makes the link with `symlinkat`. The system starts a relative `target` in the folder of the link. Gives 0, or -1. |

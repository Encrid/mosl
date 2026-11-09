# src/windows/folder_read.c

On Windows, this file lists the entries of a folder of `include/os.h`, and reads a file of the folder and its size.
Each function keeps the contract of `docs/extern/os.h.md`, *Functions*.

## The entries

`os_folder_read_entry` gives the waiting first entry, then each entry of `FindNextFileW`. It skips `.` and `..`.
It writes the name in UTF-8 into `name` of the folder, which holds 768 bytes. A name of 255 UTF-16 units needs at most 765.
`FILE_ATTRIBUTE_DIRECTORY` tells if the entry is a folder.

## The read of a file

`join_name` writes a backslash and the name after the whole path, in the room that the path keeps.
A name of more than 255 characters does not fit, and the join fails. No file of Windows has such a name.
After the call, the zero byte goes back to the end of the whole path.

`os_folder_read_file` joins the name. `_wfopen` opens the file by that path, and `_fstat64` gives its size. `_wfopen` refuses a folder.
`os_read_stream` reads the bytes: `docs/intern/common/read_stream.c.md`, *The buffer*.

## The size of a file

`os_folder_entry_size` joins the name. `os_get_whole_info` of `src/windows/entry.c` then reads the information of the entry, and follows a link: `docs/intern/windows/entry.c.md`, *The entry*.
It gives -1 for a folder, for a link with no target, and for a name that does not fit. Else `nFileSizeHigh` and `nFileSizeLow` give the size.

## Functions

| Function | Result |
| --- | --- |
| `join_name(f, name, at)` | Joins `name` to the whole path at `at`, as *The read of a file* tells. Gives false if the name does not fit. |
| `os_folder_read_entry(f, is_folder)` | Gives the next name of the search, and its kind, as *The entries* tells. Gives `nullptr` at the end. |
| `os_folder_read_file(f, name, len)` | Gives the bytes of the file `name` of *The read of a file*, or `nullptr` if `_wfopen` or the read fails. |
| `os_folder_entry_size(f, name, size)` | Writes the size of *The size of a file* into `*size`, and gives 0. Gives -1 if the name does not fit, if `os_get_whole_info` does not give 1, or if the entry is a folder. |

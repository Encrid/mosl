# src/windows/os_types.h

This header gives `include/os.h` its thread, its mutex, its event, its open folder, its watch and the state of its signals on Windows.
It defines `WIN32_LEAN_AND_MEAN`, and includes `<stdatomic.h>`, `<windows.h>` and `include/os.h`. Each other file of `src/windows` includes this header.

## Constants

`OS_NAME_ROOM` is 256: the room for a name of 255 characters and its end.
The fields `whole` and `name` of `struct os_folder` keep that room for a name.

| Constant | Value |
| --- | --- |
| `OS_CHANGES_ROOM` | 65536: the bytes of the changes of a watch. The system also keeps the changes between two reads in a buffer of this size. |
| `OS_ABOVE_ROOM` | 1024: the bytes of the changes of the folder above a watch, which no function reads. |
| `OS_NAME_BYTES` | The bytes of the longest name of a watch in UTF-8, and its end: 3 bytes for each wide character of `OS_CHANGES_ROOM`. |

## Types

### struct os_thread

A thread that `os_thread_start` makes.

| Field | Content |
| --- | --- |
| `handle` | The `HANDLE` of the thread. |
| `fn` | The function that the thread runs. |
| `arg` | The argument of `fn`. |

### struct os_mutex

A mutex: a critical section.

| Field | Content |
| --- | --- |
| `section` | The `CRITICAL_SECTION`. |

### struct os_event

An event: an event object of Windows, which resets itself as a wait returns for it.

| Field | Content |
| --- | --- |
| `handle` | The `HANDLE` of the event object. |

### struct os_folder

The open folder of `os_folder_open`, on Windows.

| Field | Content |
| --- | --- |
| `find` | The handle of the search of `FindFirstFileExW`, or `INVALID_HANDLE_VALUE`. |
| `data` | The entry that the search found last. |
| `pending` | True while the first entry of the search is not given yet. |
| `whole` | The whole path of the folder in the form `\\?\`, with room after it for a backslash and a name. A path that has the form `\\.\` keeps it. |
| `bytes` | The memory of the file read last, or `nullptr` before the first read. |
| `name` | The last name that `os_folder_read_entry` gave, in UTF-8. |

### struct os_watch

The watch of `os_watch_start`, on Windows: `docs/intern/windows/folder_watch.c.md`, *The reads*.

| Field | Content |
| --- | --- |
| `folder` | The handle of the folder, or `INVALID_HANDLE_VALUE`. |
| `above` | The handle of the folder above it, or `INVALID_HANDLE_VALUE` for a root. |
| `changes` | The `OVERLAPPED` of the read of `folder`. |
| `above_changes` | The `OVERLAPPED` of the read of `above`. |
| `final` | The path of the folder that `GetFinalPathNameByHandleW` gave at the start. |
| `again` | The room for that path at a later check, in the same block as `final`. |
| `depth` | The depth of the watch. |
| `at` | The byte of the next change in `records`. |
| `got` | The bytes of the changes in `records`. |
| `reading` | True while the read of `folder` runs. |
| `looking` | True while the read of `above` runs. |
| `stopped` | True after the watch stopped. |
| `records` | The changes of `folder`, as `FILE_NOTIFY_INFORMATION` records. |
| `above_records` | The changes of `above`. |
| `name` | The last name that `os_watch_take` gave, in UTF-8. |

### struct os_stop

The state of the signals: `docs/intern/windows/signal.c.md`, *The state*.

| Field | Content |
| --- | --- |
| `caught` | True after the first event. |
| `line` | The line that a thread writes at the first event. |
| `length` | The number of bytes of `line`. |

### os_stream_t

A pipe that the process writes into: its output or its errors.

| Field | Content |
| --- | --- |
| `pipe` | The end of the parent, or `nullptr` when closed. |
| `read` | The `OVERLAPPED` of the read that waits, with its event. |
| `pending` | True while a read waits. |
| `bytes` | The 65536 bytes of the last read, `OS_PROCESS_BYTES`. |

### os_launch_t

The texts and the ends that `CreateProcessW` needs.

| Field | Content |
| --- | --- |
| `program` | The whole path of the program. |
| `line` | The command line. |
| `block` | The environment block. |
| `folder` | The folder of the process, or `nullptr`. |
| `child` | The ends of the child: input, output, errors. |

### struct os_process

A process that `os_process_start` starts.

| Field | Content |
| --- | --- |
| `process` | The handle of the process. |
| `input` | The end of the parent of the input, or `nullptr` when closed. |
| `stop` | The event that `os_process_kill` sets. |
| `writing` | The event of a write. |
| `console_input` | The end of the parent of the input of the console. |
| `console` | The pseudo console, or `nullptr`. |
| `signals` | `signals` of the start. |
| `hung` | True after a hang-up. |
| `turn` | The count of the reads, which picks the first pipe of a read. |
| `streams` | The output and the errors. |

## Functions

The header declares the reads of a watch, which `src/windows/watch_read.c` defines.
It also declares the turns of a text that `src/windows/wide.c` defines, and `os_write_all` of `src/windows/write_all.c`.
The files of a process define the other functions of the table.

| Function | Result |
| --- | --- |
| `os_read_changes(w)` | The read of `folder`. |
| `os_read_above(w)` | The read of `above`. |
| `os_is_in_place(w)` | The check of the path of `folder`. |
| `os_finish_read(w)` | The result of the read of `folder`. |
| `os_end_read(handle, overlapped)` | The end of a read at the stop. |
| `os_widen(text)` | The copy of a text of UTF-8 in UTF-16. |
| `os_narrow(wide)` | The copy of a text of UTF-16 in UTF-8. |
| `os_widen_texts(texts)` | The copies in UTF-16 of a list of texts of UTF-8. |
| `os_free_texts(texts)` | The free of such copies. |
| `os_write_all(handle, bytes, len)` | The write of each byte into a handle. |
| `os_whole_text(path)` | The whole path of `path` in UTF-8, which `src/windows/path.c` defines. |
| `os_get_variable(name)` | The value of a variable, which `src/windows/env.c` defines. |
| `os_remove_entry(whole)` | The remove of the entry of a whole path, which `src/windows/remove.c` defines. |
| `os_get_whole_info(whole, info)` | The attributes and the size of what is at a whole path, which `src/windows/entry.c` defines. |
| `os_find_text(name)` | The path of a program on the `PATH`, which `src/windows/find.c` defines. |
| `os_find_program(start)` | The whole path of the program of a process in UTF-16. |
| `os_prepare_launch(start, launch)` | The texts of a start. |
| `os_join_line(argv)` | The command line of a process. |
| `os_build_block(env, unset)` | The environment block of a process. |
| `os_open_pipe(p, index, inbound, ends)` | The open of a pipe of a process. |
| `os_make_console(p)` | The pseudo console of a process. |
| `os_get_wait(deadline)` | The milliseconds of a wait. |
| `os_close_stream(s)` | The end of a stream, also while a read waits. |
| `os_drop_process(p)` | The free of each part of a process, which `src/windows/process_end.c` defines. |

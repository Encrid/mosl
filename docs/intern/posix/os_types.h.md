# src/posix/os_types.h

This header gives the thread, the mutex, the event, the open folder and the signal state of `include/os.h` on Linux and macOS.
It includes `<dirent.h>`, `<pthread.h>`, `<stdatomic.h>` and `include/os.h`, and the other files of `src/posix` include it.

## Types

### struct os_thread

A thread that `os_thread_start` makes.

| Field | Content |
| --- | --- |
| `handle` | The `pthread_t` of the thread. |
| `fn` | The function that the thread runs. |
| `arg` | The argument of `fn`. |

### struct os_mutex

A mutex.

| Field | Content |
| --- | --- |
| `mutex` | The `pthread_mutex_t`. |

### struct os_event

An event: a flag, the mutex that guards the flag, and a condition for a wait.

| Field | Content |
| --- | --- |
| `lock` | The `pthread_mutex_t` that guards `set`. |
| `cond` | The `pthread_cond_t` that a wait sleeps on. |
| `set` | 1 while the event is set, and 0 when it is not. |

### struct os_folder

An open folder, which `os_folder_open` makes and `os_folder_close` frees.

| Field | Content |
| --- | --- |
| `dir` | The `DIR` stream of the folder. Its descriptor opens the files of the folder. |
| `bytes` | The buffer of the last file that the folder read, or `nullptr`. |

### struct os_stop

The state of the signals: `docs/intern/posix/signal.c.md`, *The state*.

| Field | Content |
| --- | --- |
| `caught` | True after the first signal. |
| `line` | The address of the line that the handler writes at the first signal. |
| `length` | The number of bytes of `line`. |

## Functions

The header declares the two functions of a condition that `src/posix/event.c` calls.
`src/linux/event_clock.c` defines them for Linux, and `src/macos/event_clock.c` for macOS.
It also declares `os_open_search` for the walk of a long path. A file of `src/linux` and a file of `src/macos` define it.
`src/posix/folder_open.c` defines `os_open_parent` and `os_close_parent`, and `src/posix/write_all.c` defines `os_write_all`.

| Function | Result |
| --- | --- |
| `os_cond_start(cond)` | Starts the condition of an event. Gives 0 on success. |
| `os_cond_wait(e, nanos)` | Waits on the condition of `e` until its flag is set or `nanos` nanoseconds pass. The caller holds the mutex of `e`. |
| `os_open_search(at, name)` | Opens the folder `name` of the folder `at` for search only, and gives its descriptor, or -1. |
| `os_open_parent(path, name, make)` | The folder before the last name, for `src/posix/folder_open.c`, `src/posix/folder.c` and `src/posix/file.c`. |
| `os_close_parent(at)` | Closes a folder that `os_open_parent` gave. |
| `os_write_all(fd, bytes, len)` | Writes each byte into `fd` with `SIGPIPE` blocked. |

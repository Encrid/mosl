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

## Constants

`OS_PIPE_INPUT`, `OS_PIPE_OUTPUT`, `OS_PIPE_ERRORS`, `OS_PIPE_GATE` and `OS_PIPE_REPORT` are the places of the pipes of a start, 0 to 4. `OS_PIPES` is 5.
`OS_PROCESS_BYTES` is 65536: the size of the buffer of a read.

### os_launch_t

What the child of a start needs, which the parent makes before `fork`.

| Field | Content |
| --- | --- |
| `path` | The path of the program. |
| `envp` | The environment of the process. |
| `pipes` | The ends of the 5 pipes of the start, or -1. |

### struct os_process

A process that `os_process_start` starts.

| Field | Content |
| --- | --- |
| `pid` | The number of the process. |
| `input`, `output`, `errors` | The ends of the parent of the 3 pipes, or -1 when closed. Without `errors_apart`, `errors` is -1. |
| `wake` | A pipe that `os_process_kill` writes into, so a read or a write that waits ends. |
| `exit` | The descriptor that watches the end: of the process on Linux, of a queue on macOS. |
| `signals` | `signals` of the start. |
| `hung` | True after a hang-up. |
| `ended` | True after the watch told the end of the process. |
| `taken` | True after `waitid` took the process. |
| `status` | The status, after the process was taken. |
| `turn` | The pipe that the next read asks first. |
| `lock` | The mutex of `docs/intern/posix/process_end.c.md`, *The lock*. |
| `bytes` | The 65536 bytes of the last read. |

## Functions

The header declares the two functions of a condition that `src/posix/event.c` calls.
`src/linux/event_clock.c` defines them for Linux, and `src/macos/event_clock.c` for macOS.
It also declares `os_open_search` for the walk of a long path. A file of `src/linux` and a file of `src/macos` define it.
`src/posix/parent.c` defines `os_open_parent` and `os_close_parent`, and `src/posix/write_all.c` defines `os_write_all`.
`src/posix/online.c` defines `os_get_online_count`, and `src/posix/remove.c` defines `os_remove_entry`.

A file of `src/linux` and a file of `src/macos` define `os_open_pipe`, `os_quiet_pipe`, `os_watch_exit` and `os_wait_exit`.

`src/posix/process_env.c` defines `os_find_program` and `os_build_env`.
`src/posix/process_child.c` defines `os_run_child`, `os_close_pipes` and `os_drop_process`.

| Function | Result |
| --- | --- |
| `os_cond_start(cond)` | Starts the condition of an event. Gives 0 on success. |
| `os_cond_wait(e, nanos)` | Waits on the condition of `e` until its flag is set or `nanos` nanoseconds pass. The caller holds the mutex of `e`. |
| `os_open_search(at, name)` | Opens the folder `name` of the folder `at` for search only, and gives its descriptor, or -1. |
| `os_open_parent(path, name, make)` | The folder before the last name of `path`. |
| `os_close_parent(at)` | Closes a folder that `os_open_parent` gave. |
| `os_write_all(fd, wake, bytes, len)` | Writes each byte into `fd` with `SIGPIPE` blocked, and stops at a byte of `wake`. |
| `os_open_pipe(ends)` | The open of a pipe whose ends a child does not get. |
| `os_quiet_pipe(fd)` | The end of the signal of a broken write into `fd`, where the system needs it. |
| `os_watch_exit(p)` | The start of the watch of the end of a process. |
| `os_wait_exit(p, wait)` | The wait for the end of a process, up to `wait` milliseconds. |
| `os_pidfd_open(pid)` | The descriptor of a process on Linux, which `src/linux/pidfd.c` defines. |
| `os_get_wait(deadline)` | The milliseconds of a wait, which `src/posix/process_read.c` defines. |
| `os_find_program(name)` | The path of the program of a process. |
| `os_build_env(env, unset)` | The environment of a process. |
| `os_run_child(start, launch)` | The steps of a child after `fork`. |
| `os_close_pipes(pipes, count)` | The close of the pipes of a start. |
| `os_drop_process(p)` | The free of a process that did not start. |
| `os_is_pipe_waiting()` | The check of a waiting `SIGPIPE`, which `src/posix/write_all.c` defines. |
| `os_take_pipe_signal(pipe_only)` | The take of the `SIGPIPE` of a broken write. A file of `src/linux` and a file of `src/macos` define it. |
| `os_get_online_count()` | The count of the processors that run. |
| `os_remove_entry(at, name)` | The remove of the entry `name` of the folder `at`. |

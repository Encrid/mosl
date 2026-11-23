# include/os.h

This header gives the calls of the operating system, in the same form on each system.
That is address space, shared libraries, threads, the clock, a mutex, an event, and folders, watches, paths, locks and signals.
It also gives processes, files, the environment, the base folders of the user, the output, a free port and the count of the processors.

## The layer

This layer holds the code that differs by platform, and it depends on nothing of ours.
It includes `<stddef.h>` and `<stdint.h>` and no other header.

The build makes the layer into an archive `os`. A release ships `os.a`, `include/os.h`, `leaks.toml` and `fail.toml` for each target.
Each project that uses the layer fetches them from the release and links `os.a`.
`leaks.toml` names each function that takes a resource, and the function that gives it back.
A project names the file in `[leaks] with`, so that `test --leaks` counts the resources of the layer that its code takes.
The linker takes only the functions that the binary calls, so none imports a function it does not use.

`fail.toml` names each function of the layer that can fail: each function that is `[[nodiscard]]`.
With each name, it gives the value of a failed call: `nullptr` for a pointer, and -1 for a status.
It gives 0 for `os_thread_stack_left` and for `os_get_processor_count`, whose 0 tells that the system does not know.
A project names the file in `[fail] with`, so that a case of the project can make a call of the layer fail.

`include/os.h` declares each function once.
`src/posix` holds the functions for Linux and macOS, and `src/windows` holds the same functions for Windows.
`src/linux` and `src/macos` hold the parts of `src/posix` where Linux and macOS differ.
`src/common` holds the parts that are the same on each system, in the C of the standard.

The build compiles `src/{system}/*.c`, where `system` is the key of the target in `.toolchain/toolchain.toml`.
It also compiles the files of `system_sources`, the key of the target in `.toolchain/config/targets.toml`.
So no file of the layer asks which platform it is compiled for.

## Constants

Each constant is a `constexpr` of the type that the table gives.

| Constant | Type | Content |
| --- | --- | --- |
| `OS_SECOND` | `uint64_t` | `1'000'000'000`: the nanoseconds in a second. |
| `OS_FOREVER` | `uint64_t` | `UINT64_MAX`: a wait of this many nanoseconds has no time limit. |
| `OS_INPUT` | `int` | 1: the input of a process. |
| `OS_OUTPUT` | `int` | 2: the output of a process. |
| `OS_ERRORS` | `int` | 3: the errors of a process. |
| `OS_INTERRUPT` | `int` | 1: the interrupt, as *The signals* tells. |
| `OS_TERMINATION` | `int` | 2: the termination. |
| `OS_HANG_UP` | `int` | 3: the hang-up. |

## Types

### os_thread_t

A thread that `os_thread_start` makes: `struct os_thread`.
Each platform defines the struct in its `os_types.h`.

### os_mutex_t

A mutex: `struct os_mutex`, which each platform defines.

### os_event_t

An event: `struct os_event`, which each platform defines.

### os_thread_fn_t

The function that a thread runs: `void (*)(void *arg)`.

### os_folder_t

An open folder: `struct os_folder`, which each platform defines.
It lists the entries of the folder, reads its files by their names, and gives their sizes.

### os_watch_t

A watch of a folder: `struct os_watch`, which each platform defines.
It tells which entries of the folder changed, as *The watch* tells.

### os_process_t

A process that `os_process_start` starts: `struct os_process`, which each platform defines.
It holds the pipes of the process, and on Windows its console, as *The processes* tells.

### os_lock_t

A lock of a file that `os_lock` takes: `struct os_lock`, which the layer defines.
`os_unlock` ends it, as *The locks* tells.

### os_process_start_t

What `os_process_start` needs: `struct os_process_start`.

| Field | Type | Content |
| --- | --- | --- |
| `argv` | `const char *const *` | The arguments, with `nullptr` after the last. `argv[0]` names the program. |
| `folder` | `const char *` | The working folder of the process. |
| `env` | `const char *const *` | The variables that change, each as `NAME=value`, with `nullptr` after the last. |
| `unset` | `const char *const *` | The names of the variables that go, with `nullptr` after the last. |
| `errors_apart` | `bool` | `true`: the errors go into a pipe of their own. `false`: they go into the pipe of the output. |
| `signals` | `bool` | `true`: `os_process_signal` can reach the process. `false`: each signal to it gives -1. |
| `ignore_interrupt` | `bool` | `true`: the process starts with the interrupt ignored. `false`: it starts with the default action of the interrupt. |

### enum os_file_flag

The flags of `os_file_write`: an `enum os_file_flag : uint32_t` of bits. A call gives their sum, or 0 for none, as `uint32_t flags`.

| Flag | Value | Effect |
| --- | --- | --- |
| `OS_FILE_EXECUTABLE` | 1 | The file gets execute bits, as *The files* tells. |
| `OS_FILE_DURABLE` | 2 | The call flushes the bytes to the disk before the close, as *The files* tells. |

### enum os_move_flag

The flags of `os_path_move`: an `enum os_move_flag : uint32_t` of bits, given as `uint32_t flags`, or 0 for none.

| Flag | Value | Effect |
| --- | --- | --- |
| `OS_MOVE_DURABLE` | 1 | The call waits for the disk before it returns, as *The files* tells. |

A bit that is not a flag of its function gives -1, and the call does nothing. Thus a later flag never has a silent meaning in an older layer.

### enum os_folder_kind

The kinds of `os_folder_base`: an `enum os_folder_kind : uint32_t`, given as `uint32_t kind`.

| Kind | Value | Folder |
| --- | --- | --- |
| `OS_FOLDER_USER` | 1 | The folder of the data that goes with the user, for example the saves of a game. |
| `OS_FOLDER_LOCAL` | 2 | The folder of the data that stays on this machine. |

A different value gives `nullptr`. On Linux and macOS, the 2 kinds give the same folder: *The base folders*.

## Functions

Each function that can fail is `[[nodiscard]]`: the caller must use its result.

| Function | Result |
| --- | --- |
| `os_reserve` | Reserves `reserve` bytes of address space, and makes the first `commit` bytes readable, writable and zero. The system rounds `commit` up to whole pages. The rest, from the next page on, faults when touched, which a caller can use as a guard after a block. Gives `nullptr` on failure. |
| `os_commit` | Makes the first `bytes` of a reservation readable and writable. A byte that no earlier commit gave is zero. The system rounds `bytes` up to whole pages. Thus a caller grows its memory, up to the reservation. Gives 0 on success. |
| `os_release` | Gives back all of a reservation. Each caller passes a base that `os_reserve` gave. |
| `os_lib_open` | Opens a shared library, or gives `nullptr`. For a path, the loader opens the library at that path. For a bare name, the loader uses the search of the operating system. *The libraries* tells where the loader looks for what the library needs. |
| `os_lib_get_symbol` | Gives where a library has what it exports as `name`, or `nullptr`. |
| `os_lib_close` | Closes a library. Each caller passes a library that `os_lib_open` gave. |
| `os_lib_pin` | Keeps the open library of `path` in the process until its end. A later `os_lib_close` closes the handle, and the library stays. A second pin of a library changes nothing. A process can pin any number of libraries. Gives 0, or -1 if no library of `path` is open. |
| `os_thread_start` | Starts a thread that runs `fn(arg)` on a stack of at least `stack` bytes. The system can give more, because it rounds the size up to its unit and to its least stack. On Linux, glibc can also give the larger stack of a thread that ended. It also keeps a small part of the stack for itself. Gives `nullptr` on failure, for example for a stack that does not fit in the address space. |
| `os_thread_join` | Waits for a thread to end, and frees what `os_thread_start` gave. |
| `os_thread_stack_left` | Gives the bytes of stack that the calling thread can still use below the call, as *The stack of a thread* tells. Gives 0 if the system does not tell the stack of the thread. |
| `os_get_thread_id` | Gives a number for the calling thread that no other live thread has. It is never 0. |
| `os_get_now` | Gives nanoseconds from a clock that only rises. On each target, its step is below a microsecond. |
| `os_mutex_create` | Makes a mutex. Gives `nullptr` on failure. |
| `os_mutex_free` | Frees a mutex that no thread holds. A null `m` does nothing. |
| `os_mutex_lock` | Takes a mutex, and waits while another thread holds it. |
| `os_mutex_unlock` | Gives a mutex back. |
| `os_event_create` | Makes an event that one thread sets and another waits for. A set before the wait is not lost, and a wait consumes it. Gives `nullptr` on failure. |
| `os_event_free` | Frees an event that no thread waits for. A null `e` does nothing. |
| `os_event_set` | Sets an event: one wait, now or later, returns for it. |
| `os_event_wait` | Waits until the event is set or `nanos` nanoseconds pass, and consumes a set. The clock of `os_get_now` measures the nanoseconds, so a step of the wall clock does not change a wait. On Windows, the wait rounds `nanos` up to whole milliseconds. With `OS_FOREVER` it has no time limit. |
| `os_folder_set` | Moves the process into `folder`, so that relative paths start in `folder`. Gives 0 on success. |
| `os_folder_make` | Makes `folder`, and each folder above it that is not there. Gives 0 on success, also when `folder` is there already. |
| `os_folder_open` | Opens the folder `path`, so that the caller can list it and read its files, as *The folders* tells. Gives `nullptr` if no folder of `path` opens. |
| `os_folder_read_entry` | Gives the name of the next entry of the folder, and sets `*is_folder` to true if the entry is a folder. It skips `.` and `..`. The entries come in the order of the file system, which differs by system and by file system. Thus a caller that needs an order sorts the names. The name stays valid until the next call. Gives `nullptr` after the last entry. |
| `os_folder_read_file` | Reads the whole file `name` of the folder. It gives the bytes, then one zero byte, and sets `*len` to the count of the bytes. The bytes belong to the folder: they stay valid until its next read or its close, and the caller does not free them. `name` is one name of the folder, with no separator. Gives `nullptr` if the file does not open or read, or if it is a folder. |
| `os_folder_entry_size` | Gives 0, and sets `*size` to the size in bytes of the file `name`, as *The folders* tells. It reads no byte of the file. `name` is one name of the folder, with no separator. Gives -1 and keeps `*size` if no file of `name` is there, if it is a folder, or if the call fails. |
| `os_folder_close` | Closes a folder, and frees its bytes. A null `f` does nothing. |
| `os_watch_start` | Starts a watch of the entries of `folder`, down to `depth` levels of folders, as *The watch* tells. Gives `nullptr` if `depth` is 0, if no folder of `folder` opens, or if a call that it needs fails. |
| `os_watch_take` | Gives the next change of a watch, and does not wait. It gives 1 and sets `*name`, or it gives 0, 2 or -1, as *The watch* tells. |
| `os_watch_stop` | Stops a watch, and frees it. A null `w` does nothing. |
| `os_path_whole` | Gives the absolute path of `path`, which the caller frees, or `nullptr`. |
| `os_lock` | Takes an exclusive lock on the file `path`, so that one process at a time holds it, as *The locks* tells. It does not wait. It gives 0 and sets `*lock` on success, 1 if a lock holds the file already, and -1 on a failure. |
| `os_unlock` | Ends the lock of `lock`, and frees it. A null `lock` does nothing. |
| `os_signal_catch` | Catches the signals that stop the process, and writes `line` at the first signal, as *The signals* tells. Gives 0 on success. |
| `os_signal_is_caught` | Tells if a signal set the flag of `os_signal_catch`. A caller reads it between two parts of its work, and then ends its work. |
| `os_process_start` | Starts a program with the fields of `start`, as *The processes* tells. Gives `nullptr` if the program does not start. The streams of the caller then stay as they are. |
| `os_process_write` | Writes all the `len` bytes into the input of the process. Gives 0 on success, and -1 if the input is closed or the write fails. |
| `os_process_close` | Closes one pipe of the process: `OS_INPUT`, `OS_OUTPUT` or `OS_ERRORS`. |
| `os_process_read` | Gives the next bytes of the output or of the errors, and waits up to `nanos` nanoseconds for them, as *The processes* tells. |
| `os_process_signal` | Sends `OS_INTERRUPT`, `OS_TERMINATION` or `OS_HANG_UP` to the process, as *A signal to a process* tells. Gives 0 on success. |
| `os_process_kill` | Ends the process at once, as *The processes* tells. |
| `os_process_wait` | Waits up to `nanos` nanoseconds for the end of the process, and gives its status, as *The processes* tells. |
| `os_process_free` | Frees a process. It kills a process that still runs, and waits for its end. A null `p` does nothing. |
| `os_file_write` | Writes all the bytes of the file `path`, with the flags of `enum os_file_flag`, as *The files* tells. With `OS_FILE_DURABLE`, it flushes the bytes to the disk before the close. On macOS, the cache of the drive can still hold the bytes. Gives 0 on success, and -1 for a flag that is not known. |
| `os_path_entry` | Tells what is at `path`, as *The files* tells. |
| `os_path_move` | Gives the file or the folder `from` the path `to`, with the flags of `enum os_move_flag`, as *The files* tells. With `OS_MOVE_DURABLE`, it waits for the disk before it returns. On macOS, the cache of the drive can still hold the move. Gives 0 on success, and -1 for a flag that is not known. |
| `os_path_remove` | Removes what is at `path`, as *The files* tells. |
| `os_link_make` | Makes `path` a symbolic link to `target`, as *The files* tells. Gives 0 on success. |
| `os_file_link` | Gives the file `from` the second name `to`, a hard link, as *The files* tells. Gives 0 on success, and -1 else. |
| `os_env_get` | Gives a copy of the value of the variable `name`, as *The environment* tells. |
| `os_path_find` | Finds the program `name`, as *The environment* tells. |
| `os_folder_base` | Gives the base folder of the data of the user for `kind`, which the caller frees, as *The base folders* tells. Gives `nullptr` for a kind that is not known, or if the system gives no folder. |
| `os_get_program_path` | Gives the absolute path of the program of the process, which the caller frees, or `nullptr`. |
| `os_output_write` | Writes all the `len` bytes to the standard output, as *The output* tells. Gives 0, or -1 if a write fails. |
| `os_get_free_port` | Gives a port of TCP on `127.0.0.1` that no socket held at the call, or -1. Another program can take the port after the call. On Windows, the layer loads `ws2_32.dll` at the call, so a program that links `os.a` links no other library. |
| `os_get_processor_count` | Gives the count of the processors that the process can use, or 0 if the system does not tell it. |

## The stack of a thread

`os_thread_stack_left` counts from its own frame down to the last byte of the stack that the thread can use.
Thus a caller can refuse a call that needs more stack before the call ends the process.
The caller keeps room for the frames that run between its check and that call.
The count is for the stack that the system gave the thread, on the first thread of the process and on each other thread.

- On Linux, the first call of a thread asks glibc with `pthread_getattr_np`, which makes system calls. The thread keeps the end of its stack, so each later call reads only that end.
- If `pthread_getattr_np` fails, the call gives 0, and the next call of the thread asks again.
- On macOS, each call reads the stack of the thread with `pthread_get_stackaddr_np` and `pthread_get_stacksize_np`, with no system call.
- On Windows, each call reads the limits of the stack and the stack guarantee of the thread, with no system call.
- On Windows, a call that reaches the lowest 16 KiB of the stack ends the process. With a stack guarantee of more than 8 KiB, that part is the guarantee and 8 KiB. The count leaves out that part.

## The folders

A folder opens by its path, and `os_folder_read_file` reads a file of it by its name. Thus a file can be past the path limit of the system:

- On Linux and macOS, `os_folder_read_file` opens the name from the folder with `openat`, so the path of the file is never joined.
- `os_folder_entry_size` takes the name in the same way. It follows a link, and gives the size of its target. A link to a folder or to nothing gives -1.
- A path of a folder past `PATH_MAX` opens one name at a time with `openat`. `PATH_MAX` is 4096 bytes on Linux, and 1024 on macOS.
- Each folder of that path before the last opens for search only: with `O_PATH` on Linux, and with `O_SEARCH` on macOS. Thus a long path needs the same rights as a short one.
- On Windows, `GetFullPathNameW` makes the path whole, from the current folder for a relative path. The layer then gives it the form `\\?\`, and joins the name of a file to it.
- Windows takes that form up to about 32,767 characters. A network path `\\server\share` gets the form `\\?\UNC\server\share`. A path that has the form `\\?\` or `\\.\` already keeps that form.
- `os_folder_make` also takes a path past that limit, and makes each folder of it that is not there.
- `os_folder_set` keeps the limit of each system, so the current folder stays short: `PATH_MAX` on Linux and macOS, and 260 characters on Windows. Windows needs an opt-in of the program for a longer current folder.

The path of a folder can end with a separator. It can also be a root: `/` on Linux and macOS, or the root of a drive on Windows, for example `C:\`.

While `os_folder_read_file` holds a file open, another program can write the file, delete it, or move a file onto its path.
On Windows, the layer opens the file with `FILE_SHARE_READ`, `FILE_SHARE_WRITE` and `FILE_SHARE_DELETE` for that. The read then gives the bytes of the file that it opened.

A library loads by its path, so `os_lib_open` keeps the limit of the loader of each system. That limit is 4096 bytes on Linux, 1024 on macOS, and 260 characters on Windows.

## The libraries

`os_lib_open` gives the loader of Linux and macOS no flag of search. Thus each loader looks for what a library needs in its own places:

- On Windows, `LoadLibraryExW` with `LOAD_WITH_ALTERED_SEARCH_PATH` looks first in the folder of a library that a path names. Thus a library can bring its runtime beside itself.
- On Linux, a run path of `$ORIGIN` in the library makes the loader look in the folder of the library. A `LD_LIBRARY_PATH` that names the folder does it too.
  Else the loader uses the other rules of `dlopen`: the cache of `ldconfig`, and `/lib` and `/usr/lib`.
- On macOS, a needed name that starts with `@loader_path` makes the loader look in the folder of the library.
  A name that starts with `@rpath` does it too, if the library itself has the run path `@loader_path`.
  The loader also reads the run paths of each image that loaded the library, also of the program. There, `@loader_path` is the folder of that image.
  A name with `@executable_path`, an absolute name, `DYLD_LIBRARY_PATH` or `DYLD_FALLBACK_LIBRARY_PATH` can also lead to the folder of the library.
- Thus on Linux and macOS, the build of a library that brings its runtime beside itself gives the library that run path or that name.

The sources of the search:

- Linux, `dlopen`: dependencies "are also automatically loaded by the dynamic linker using the same rules". https://man7.org/linux/man-pages/man3/dlopen.3.html
- Linux, `dlopen`: the rules are `DT_RPATH`, `LD_LIBRARY_PATH`, `DT_RUNPATH`, "The cache file /etc/ld.so.cache", and "The directories /lib and /usr/lib".
- Linux, `ld.so`: `$ORIGIN` "expands to the directory containing the program or shared object." https://man7.org/linux/man-pages/man8/ld.so.8.html
- Apple, `dyld`, in a copy of the man pages of Xcode: https://keith.github.io/xcode-man-pages/dyld.1.html
- Apple, `dyld`: "The run path stack is built from the LC_RPATH load commands in the dependency chain that lead to the current dylib load."
- Apple, `dyld`: "in every binary, @loader_path resolves to a different path, whereas @executable_path always resolves to the same path."
- Apple, `dyld`: with `DYLD_LIBRARY_PATH`, "the dynamic linker looks for its leaf name in each directory". With `DYLD_FALLBACK_LIBRARY_PATH`, "If a dylib is not found at its install path, dyld uses this as a list of directories to search".
- Microsoft, `LoadLibraryExW`: with an absolute path, "the system uses the alternate file search strategy" "to find associated executable modules". https://learn.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-loadlibraryexw

## The watch

A watch tells which entries of a folder changed. Thus a caller reads only these entries, and not all the entries of the folder.
At a depth of 1, the watch tells the entries of `folder`. At 2, it also tells the entries of each folder in it, and so on.

`os_watch_take` gives one of these results:

| Result | Meaning |
| --- | --- |
| 1 | `*name` is the path of an entry that was made, changed, moved or removed. The path starts in `folder`, and has a `/` between two names. It stays valid until the next call of `os_watch_take` or `os_watch_stop`. |
| 0 | No change is left. |
| 2 | The watch lost changes. The caller looks at all of `folder` again, and the watch goes on. |
| -1 | The watch stopped, for example because `folder` was removed or moved. Each later call also gives -1. |

- An entry can come more than once. A move gives the old name and the new name.
- A name can be a folder. Then the entries in that folder can have changed too, with no names of their own. For example, a folder that moves in brings its entries. Thus the caller looks at that folder.
- No path has more names than `depth`.
- One thread at a time uses a watch.

The watch lets other programs remove or move `folder`. A file system that refuses notices of change makes `os_watch_start` give `nullptr`.
On Linux, a watch of a network file system does not tell the changes that other machines make. On macOS, it can miss them.

Each system tells the changes in its own way:

- On Windows, `ReadDirectoryChangesW` tells the changes, and a second one watches the folder above `folder` for its move. The root of a drive cannot move, and has no folder above it.
- On Linux, `inotify` watches each folder down to the depth. A folder that comes in gets its watch before `os_watch_take` gives its name. `max_user_instances` and `max_user_watches` of the system limit the count of the watches and of their folders. A folder whose path is longer than `PATH_MAX` cannot be watched.
- On macOS, FSEvents tells the changes. The layer loads `CoreServices` when it starts a watch, so a program that links `os.a` links no framework. FSEvents can tell a change from just before the start, for example the removal of an earlier folder with the same path.

A watch gives 2 when the changes between two takes do not fit. It keeps 64 KiB on Windows and on macOS, and `max_queued_events` changes on Linux. FSEvents can also lose changes.
A take with no change makes no call of the system on Windows and on macOS, and one call of `read` on Linux.

The names come in the order in which the system hands them over.
On Linux, that is the buffer of `read`. On Windows, it is the chain of the records of `ReadDirectoryChangesW`. On macOS, it is the arrays of the callback of FSEvents.
Only Linux states that this order is the order of the changes:

- On Linux, the names come in the order of the changes, since inotify gives its events as an ordered queue.
- On Windows, `ReadDirectoryChangesW` states no order of its records. Thus a caller does not rely on the order of 2 names.
- On macOS, the ids of the events increase, but FSEvents states no order of the events of a callback. It can also merge the changes of 1 folder that come close in time. Thus a caller does not rely on the order of 2 names.

- The Linux man-pages project, `inotify(7)`: https://man7.org/linux/man-pages/man7/inotify.7.html
- `inotify(7)`: "The events returned by reading from an inotify file descriptor form an ordered queue."
- Microsoft, `ReadDirectoryChangesW`: https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-readdirectorychangesw
- Microsoft, `ReadDirectoryChangesW`: "Directory changes that occur between calls to this function are added to the buffer and then returned with the next call."
- Apple, File System Events Programming Guide, the use of the API: https://developer.apple.com/library/archive/documentation/Darwin/Conceptual/FSEvents_ProgGuide/UsingtheFSEventsFramework/UsingtheFSEventsFramework.html
- Apple, the use of the API: a stream of 1 host holds "events whose IDs are increasing".
- Apple, File System Events Programming Guide, the overview: https://developer.apple.com/library/archive/documentation/Darwin/Conceptual/FSEvents_ProgGuide/TechnologyOverview/TechnologyOverview.html
- Apple, the overview: the daemon "coalesces multiple notifications on a given directory if they occur in a short period of time".

## The locks

`os_lock` opens the file `path`, and makes it if it is not there. Then it takes an exclusive lock of the file, and does not wait.
On Linux and macOS, the lock is advisory: it stops only a different `os_lock` of the file, and not a read or a write.
On Windows, the lock also refuses a read or a write of the first byte through a different handle.

| Result of `os_lock` | Meaning |
| --- | --- |
| 0 | The caller holds the lock, and `*lock` is its handle. |
| 1 | A lock holds the file already: of a different process, or of an earlier call of this process. `*lock` is `nullptr`. |
| -1 | The file does not open, the memory of the handle fails, or the system refuses the lock for a different reason. `*lock` is `nullptr`. |

Only a lock that holds the file gives 1. Thus a caller can tell a held file from a failure.
For example, `ENOLCK` of `flock` on Linux gives -1: the kernel has no memory for the lock.
On Windows, `ERROR_ACCESS_DENIED` of `LockFileEx` gives -1, also though `_locking` then sets `errno` to `EACCES`.

`os_unlock` ends the lock, closes the file and frees the handle. A later `os_lock` of the file, also in the same process, then takes the lock.
The file stays, because a different process can wait to lock it.

The end of a process also ends each lock that it holds. A program that the process starts does not get the file of the lock, also with a call other than `os_process_start`. Thus the lock ends with the process that took it.
On Windows, the end of the lock after the end of the process can take some time. Thus a program calls `os_unlock` before its end.

- On Linux and macOS, `flock` takes `LOCK_EX` with `LOCK_NB`, and `os_unlock` gives `LOCK_UN`, then `close`. Only the error `EWOULDBLOCK` of `flock` gives 1.
- On Windows, `_locking` locks the first byte with `_LK_NBLCK`, and `os_unlock` gives `_LK_UNLCK`, then `_close`. Only the error `ERROR_LOCK_VIOLATION` in `_doserrno` gives 1.
  The C library of Windows gives `EACCES` also for other refusals of the system, for example `ERROR_ACCESS_DENIED`. Thus `os_lock` does not read `errno` on Windows.

The sources of a lock:

- Linux, `flock`: "Locks created by flock() are associated with an open file description". https://man7.org/linux/man-pages/man2/flock.2.html
- Linux, `flock`: a lock "may be denied by a lock that the calling process has already placed via another file descriptor."
- Linux, `flock`: the lock is released "when all such file descriptors have been closed."
- Linux, `flock`: `EWOULDBLOCK` tells "The file is locked and the LOCK_NB flag was selected."
- Linux, `flock`: `ENOLCK` tells "The kernel ran out of memory for allocating lock records."
- Microsoft, `_locking`: `_LK_UNLCK` "Unlocks the specified bytes, which must have been previously locked." https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/locking
- Microsoft, `_locking`: `EACCES` tells a "Locking violation (file already locked or unlocked)."
- Microsoft, `_doserrno`: "For I/O operations, `_doserrno` stores the operating-system error-code equivalents of `errno` codes." https://learn.microsoft.com/en-us/cpp/c-runtime-library/errno-doserrno-sys-errlist-and-sys-nerr
- Microsoft, System Error Codes: `ERROR_LOCK_VIOLATION` tells "The process cannot access the file because another process has locked a portion of the file." https://learn.microsoft.com/en-us/windows/win32/debug/system-error-codes--0-499-
- Microsoft, the source of the UCRT in the Windows SDK, lowio/locking.cpp: on a failure of `LockFileEx`, `_locking` calls `__acrt_errno_map_os_error(GetLastError())`.
- The same source, misc/errno.cpp: that function sets `_doserrno = oserrno`, and maps `ERROR_ACCESS_DENIED`, `ERROR_LOCK_VIOLATION` and the codes 19 to 36 to `EACCES`.
- Microsoft, `LockFile`: when a process ends with a lock, "the locks are unlocked by the operating system." https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-lockfile
- Microsoft, `LockFile`: "However, the time it takes for the operating system to unlock these locks depends upon available system resources."
- Microsoft, `LockFile`: with a second handle, the locking process "cannot access the specified region through this second handle until it unlocks the region."
- Microsoft, `_locking`: "Locking bytes in a file prevents access to those bytes by other processes."

## The signals

After `os_signal_catch`, an interrupt, a termination or a hang-up sets a flag, and the process goes on.
A second signal ends the process at once, with status 1, also while the output is full.

At the first signal, the layer writes `line` to the standard output at once, also while the work of the program hangs.
`line` holds its own line end, and stays valid until the end of the process, for example as a literal.
The layer writes it with one call of the system, past the buffer of `stdio`.
So a program that keeps its lines in a buffer must flush each line as it writes it, for example with `fflush`.
Else the line of the signal can come before older lines. On Windows, `_IOLBF` gives a full buffer, so a line buffer does not do it.

On Linux and macOS, these signals are `SIGINT`, `SIGTERM` and `SIGHUP`.
A signal that the parent made the process ignore stays ignored, for example `SIGHUP` under `nohup`.
The process also ignores `SIGPIPE`. Thus a write to a closed output fails, and does not end the process.

On Windows, the signals are the console events: Ctrl-C, Ctrl-Break, the close of the console, a logoff and a shutdown.
A parent can make a process ignore only Ctrl-C, and then Ctrl-C stays ignored.
A start of a process with `signals` ends that ignore, as *The processes* tells.
After a close, a logoff or a shutdown, Windows ends the process some seconds later in any case.
Windows has no `SIGPIPE`: a write to a closed output fails.

### A signal to a process

`os_process_signal` sends a signal to a process that started with `signals`:

- On Linux and macOS, it sends `SIGINT`, `SIGTERM` or `SIGHUP` to the process only.
- On Windows, the process runs on a pseudo console of its own, of 80 columns and 25 lines.
- The interrupt writes Ctrl-C into the input of that console, and the termination writes the key Ctrl-Break. The hang-up closes the console.
- Some versions of Windows end the processes of a console at its close, and others do not. So after the close, the hang-up waits up to 1 second for the end of the process.
- If the process still runs, the layer ends it with the status 3221225786, `STATUS_CONTROL_C_EXIT`. Thus a hang-up ends the process on each version.
- Windows gives the same status when the close ends a process after the end of its start. A run with a different status in `TerminateProcess` measured it.
- On Windows, a signal reaches each process on that console: the process, and each process that it started there.
- The console drops what is written to it, because the process writes into its pipes.

Each system gives -1 for a signal to a process that started without `signals`.
Each system also gives -1 for a signal after a hang-up, because on Windows the hang-up ends the console.

On Windows, an interrupt or a termination that the caller sends before the process has finished its start can be lost.
A hang-up that the caller sends before that time ends the process, but the process does not get it as a signal.
Thus `os_signal_catch` of that process does not write its line.
A result of 0 does not tell that the process got the signal.
Thus the caller waits for a line of the output of the process, and then sends the signal.
Do not send the signal again: a process that uses `os_signal_catch` ends at once at a second signal, as *The signals* tells.

On Windows, 2 different signals that the caller sends close together can both be lost.
The console sets a flag for each key of a signal, and it sends the signal at its next dispatch.
A dispatch clears each flag, and it sends at most 1 signal.
If 2 different flags are set at a dispatch, the console sends no signal. 2 keys of the same signal give only 1 signal.

Thus the caller waits for the effect of a signal before it sends the next signal.
The effect is the line of `os_signal_catch`, or the end of the process.

On Windows, the caller cannot tell an interrupt that the process ignores from an interrupt that is lost. In each case, the interrupt has no effect.

The sources of these limits:

- OpenConsole, `VtIo.cpp`, lines 233-237, is about a close of the console before its first process has connected. https://github.com/microsoft/terminal/blob/cf5e6bf22727a251aced8bc30491c87428a4e116/src/host/VtIo.cpp#L233-L237
- `VtIo.cpp`, line 236: "but since it hasn't finished connecting yet, it won't react to a CTRL_CLOSE_EVENT."
- `VtIo.cpp`, line 237, about the same close: "Instead, we return an error here which will abort the connection setup."
- These 2 quotes are about the close only. Runs by hand on Windows 11 measured the loss of an interrupt and of a termination before the end of the start.
- No row of the tests can make a loss happen each time. Thus these limits have their sources, and no row.
- OpenConsole, `VtInputThread.cpp`, lines 106-114, takes the lock of the console for each part of the input that it reads. The unlock does the dispatch. https://github.com/microsoft/terminal/blob/cf5e6bf22727a251aced8bc30491c87428a4e116/src/host/VtInputThread.cpp#L106-L114
- `VtInputThread.cpp`, line 107: "Only the global unlock attempts to dispatch ctrl events."
- OpenConsole, `input.cpp`, lines 228-240: each key of a signal sets its flag, for example "gci.CtrlFlags |= CONSOLE_CTRL_C_FLAG;". https://github.com/microsoft/terminal/blob/cf5e6bf22727a251aced8bc30491c87428a4e116/src/host/input.cpp#L228-L240
- OpenConsole, `input.cpp`, lines 287-290: with no process to signal, the dispatch returns, and the flags stay. https://github.com/microsoft/terminal/blob/cf5e6bf22727a251aced8bc30491c87428a4e116/src/host/input.cpp#L287-L290
- OpenConsole, `input.cpp`, line 294, at the dispatch, clears each flag: "const auto CtrlFlags = std::exchange(gci.CtrlFlags, 0);". https://github.com/microsoft/terminal/blob/cf5e6bf22727a251aced8bc30491c87428a4e116/src/host/input.cpp#L294
- OpenConsole, `input.cpp`, lines 308-331: the `switch` matches only 1 flag, and with 2 flags its `default` does "return;". https://github.com/microsoft/terminal/blob/cf5e6bf22727a251aced8bc30491c87428a4e116/src/host/input.cpp#L308-L331

## The processes

`os_process_start` starts a program with the fields of `os_process_start_t`.
It keeps the limit of the system for `argv[0]` and for `folder`, as `os_folder_set` does.
Each text of `argv`, `env` and `unset` is UTF-8. On Windows, the layer turns each text into UTF-16 for the system.

The program:

- If `argv[0]` holds a separator, it is a path. A relative path starts in `folder`.
- Else `os_path_find` finds it on the `PATH` of the caller, as *The environment* tells.
- On Windows, the layer joins `argv` into one command line. The program splits it again by the rules of `CommandLineToArgvW`.

The process gets the environment of the caller, with the variables of `env`, and without the names of `unset`.
On Windows, a name has no case, so `Path` and `PATH` name one variable.

The pipes:

- The process has a pipe for its input and a pipe for its output.
- The process gets these pipes as its 3 streams, also when a stream of the caller is closed.
- With `errors_apart`, the errors have a pipe of their own. Else they go into the output, in the order that the process writes them.
- `os_process_write` waits while the pipe is full. A write into a closed input gives -1, and does not end the caller.
- After the close of the input, the process reads the end of its input. After the close of the output or of the errors, each later write of the process into that pipe fails.

`os_process_read` waits up to `nanos` nanoseconds for bytes of the output or of the errors.
It sets `*bytes` and `*len` to at most 65536 bytes, which belong to the process.
They stay valid until the next read, the close of their pipe, or the free, and the caller does not free them.
Thus one thread reads the two pipes, and no pipe fills while the thread waits for the other one.

| Result of `os_process_read` | Meaning |
| --- | --- |
| `OS_OUTPUT` or `OS_ERRORS` | The bytes came from that pipe. |
| 0 | The output and the errors ended, or are closed. |
| -1 | The read failed. |
| -2 | `nanos` passed, and no byte came. |

`os_process_wait` waits up to `nanos` nanoseconds for the end of the process. With `OS_FOREVER`, it has no time limit.
On Linux and macOS, a read and a wait count in whole milliseconds, so `nanos` rounds up to a millisecond.

| Result of `os_process_wait` | Meaning |
| --- | --- |
| 0 | The process ended, and `*status` holds its status. |
| 1 | `nanos` passed, and the process still runs. |
| -1 | The wait failed. |

The status is the exit code of the process.
On Linux and macOS, a process that a signal ended has minus the number of the signal, for example -9 for `SIGKILL`.
On Windows, the status is the exit code, from 0 to 4294967295.

`os_process_kill` ends the process at once: with `SIGKILL` on Linux and macOS, and with `TerminateProcess` and the code 1 on Windows.
It ends only the process, and not the processes that it started. A kill of a process that ended does nothing.
`os_process_kill` can come from another thread, also while a read or a write of the process waits. That read or write then ends, as at the end of the process.
Each other call of one process comes from one thread at a time.

Threads can start processes at one time.

The signals of the process at its start:

- On Linux and macOS, the process starts with no blocked signal, and with the default action of `SIGINT`, `SIGTERM`, `SIGHUP` and `SIGPIPE`.
- With `ignore_interrupt`, it starts with `SIGINT` ignored.
- On Windows, a process gets an ignored Ctrl-C from its parent. So each start with `signals` and no `ignore_interrupt` calls `SetConsoleCtrlHandler(nullptr, false)` in the caller.
- With `ignore_interrupt`, the process starts in a new process group of Windows, which ignores Ctrl-C.

## The files

Each call on a path takes a path past the limit of the system, as `os_folder_open` does: *The folders*.

`os_file_write` makes the file `path`, or replaces all its bytes. The folder of the file must be there.

- A new file gets the mode 0777 without the bits of the umask with `OS_FILE_EXECUTABLE`, and 0666 without them else.
- A file that was there keeps its read bits and its write bits. With `OS_FILE_EXECUTABLE`, it gets an execute bit for each read bit that it has. Else it loses each execute bit.
- Thus no call reads the umask: POSIX has no call that reads it with no change. A private file, for example of mode 0600, stays private.
- On Windows, a file has no execute bit, and `OS_FILE_EXECUTABLE` changes nothing.

With `OS_FILE_DURABLE`, `os_file_write` flushes the bytes of the file to the disk before it closes the file. A failed flush gives -1.
With no `OS_FILE_DURABLE`, the call does not wait for the disk, and the system writes the bytes later.

- On Windows, `FlushFileBuffers` flushes the file.
- On Linux and macOS, `fsync` flushes the file. On Linux, it also flushes the cache of the disk.
- On macOS, `fsync` gives the bytes to the drive, and the drive can keep them in its cache for some time. The call does not use `F_FULLFSYNC`.
- The flush does not sync the entry of a new file in its folder. To keep the name too, give the file its path with `os_path_move` and `OS_MOVE_DURABLE`.
- Each flush waits for the disk. Thus a durable write of each small file costs more time than one write with no `OS_FILE_DURABLE`.

The sources of a durable write:

- Microsoft, `FlushFileBuffers`: "The file handle must have the GENERIC_WRITE access right." It "can be inefficient when used after every write". https://learn.microsoft.com/en-us/windows/win32/api/fileapi/nf-fileapi-flushfilebuffers
- Linux, `fsync`: "This includes writing through or flushing a disk cache if present." https://man7.org/linux/man-pages/man2/fsync.2.html
- macOS, `fsync`: "The F_FULLFSYNC fcntl asks the drive to flush all buffered data to permanent storage." https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/fsync.2.html

`os_path_entry` follows a link, and tells what is at its end:

| Result of `os_path_entry` | Meaning |
| --- | --- |
| 0 | Nothing is at `path`. |
| 1 | A file. On Windows, each file gives 1. |
| 2 | A file with an execute bit. |
| 3 | A folder. |
| -1 | The call failed. |

`os_path_move` gives the file or the folder `from` the path `to`, and replaces a file at `to`.
The two paths must be on one volume, else the move fails.

The move also replaces a file at `to` that another program holds open:

- On Linux and macOS, `renameat` replaces the entry. An open handle keeps the old file.
- On Windows, the layer first renames with `SetFileInformationByHandle` and `FileRenameInfoEx`, with `FILE_RENAME_FLAG_REPLACE_IF_EXISTS` and `FILE_RENAME_FLAG_POSIX_SEMANTICS`. A handle that shares the delete then keeps the old file, and the path gives the new one.
- If the system refuses that rename, for example a file system with no such semantics, the layer moves with `MoveFileExW`. That move fails while another handle holds `to` open.
- On Windows, no rename replaces a file that runs, such as a library that a process loaded.

With `OS_MOVE_DURABLE`, `os_path_move` waits for the disk before it returns. On macOS, the cache of the drive can still hold the move.
With no `OS_MOVE_DURABLE`, the call does not wait for the disk, and the system writes the move later.

- On Windows, `OS_MOVE_DURABLE` adds nothing to a move on 1 volume. The rename of `FileRenameInfoEx` has no flag that waits for the disk.
- A move by `MoveFileExW` gets the flag `MOVEFILE_WRITE_THROUGH`. Microsoft states its guarantee only for a move that copies and deletes the file. `os_path_move` never copies, because it does not use `MOVEFILE_COPY_ALLOWED`.
- On Linux and macOS, `fsync` syncs the folder of `to` after the move. It also syncs the folder of `from` if that is a different folder.
- On Linux and macOS, a failed sync gives -1, and the move is made: the entry is at `to`, and not at `from`.
- On macOS, the cache of the drive can keep the entries for some time, as for `os_file_write`. The drive can also write them in a different sequence.
- Each sync waits for the disk, so a durable move costs more time than a move with no `OS_MOVE_DURABLE`.

On Linux, a durable write and a durable move keep a whole file. Write the file under a temporary path, then move it to its path.
On Windows, a durable write keeps the bytes of the file, and no source states when the new name reaches the disk.
On macOS, the drive can write the new name before the bytes. Thus a loss of power can give the new name with bytes that are not complete.

The sources of a durable move:

- Microsoft, `MoveFileExW`: "The function does not return until the file is actually moved on the disk." https://learn.microsoft.com/en-us/windows/win32/api/winbase/nf-winbase-movefileexw
- Microsoft, `MoveFileExW`: "Setting this value guarantees that a move performed as a copy and delete operation is flushed to disk before the function returns."
- Microsoft, `FILE_RENAME_INFORMATION`: with `FILE_RENAME_POSIX_SEMANTICS`, "allow replacing a file even if there are existing handles to it. Existing handles to the replaced file continue to be valid for operations such as read and write." https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/ns-ntifs-_file_rename_information
- Microsoft, `FILE_RENAME_INFORMATION`: a rename fails for a target that "is a directory, a read-only file, or a currently executing file."
- macOS, `fsync`: "it may be written in an out-of-order sequence." https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man2/fsync.2.html
- Linux, `fsync`: "Calling fsync() does not necessarily ensure that the entry in the directory containing the file has also reached disk." https://man7.org/linux/man-pages/man2/fsync.2.html
- Linux, `fsync`: "For that an explicit fsync() on a file descriptor for the directory is also needed."

`os_path_remove` removes the file, the link or the folder at `path`, with all the entries of the folder:

- It removes a read-only entry too. It removes a link, and keeps its target.
- It gives 0 when it removed `path`, 1 when nothing was at `path`, and -1 on a failure.
- After a failure, some entries can be gone.

`os_link_make` makes `path` a symbolic link to `target`. A relative `target` starts in the folder of `path`.
An empty `target` gives -1 on each system, because a link to no path is of no use.
On Windows, a link needs the Developer Mode or the rights of an administrator.
On Windows, the link is a link to a folder if `target` is a folder at the call.

`os_file_link` gives the file `from` a second name, `to`: a hard link. The 2 names hold the same bytes, and need no right of an administrator on any system.
It gives -1 if `from` is no file or a folder, if something is at `to`, or if the paths are on 2 volumes.
It also gives -1 if the folder of `to` is not there.

A write in place through 1 name changes the bytes of the other.
A move onto 1 name gives that name a new file. The other name keeps the old bytes.
On Linux and macOS, `linkat` makes the link. On Windows, `CreateHardLinkW` makes it.

## The environment

`os_env_get` gives a copy of the value of the variable `name`, which the caller frees.
It gives `nullptr` if the variable is not there, or if the copy fails.

`os_path_find` gives the absolute path of the program `name`, which the caller frees, or `nullptr`:

- A name with a separator is a path. The call gives it if it is a file with an execute bit, or on Windows a file.
- Else the call looks in each folder of the `PATH` of the caller, in order.
- On Windows, if `name` has no extension, it tries `name` with each extension of `PATHEXT`, in order, and then `name` alone. With no `PATHEXT`, it tries no extension.
- It looks in the current folder only if `PATH` names it.

## The base folders

`os_folder_base` gives the absolute path of a base folder of the data of the user. The caller frees it.
The caller adds the names of its own folders to the base, and makes them with `os_folder_make`.

| System | `OS_FOLDER_USER` | `OS_FOLDER_LOCAL` |
| --- | --- | --- |
| Windows | The known folder `FOLDERID_SavedGames`, by default `%USERPROFILE%\Saved Games`. | The known folder `FOLDERID_LocalAppData`, by default `%USERPROFILE%\AppData\Local`. |
| Linux | `XDG_DATA_HOME`, or `$HOME/.local/share`. | The same folder. |
| macOS | `$HOME/Library/Application Support`. | The same folder. |

- On Windows, `SHGetKnownFolderPath` gives the current path of the folder, also when it is redirected, because the call does not give `KF_FLAG_DEFAULT_PATH`.
- On Windows, the call gives `KF_FLAG_CREATE`, so Windows makes a folder that is not there, with its rights. Without it, a folder that is not there gives `nullptr`.
- On Windows, the layer loads `shell32.dll` and `ole32.dll` at the call, so a program that links `os.a` links no other library. `CoTaskMemFree` frees the text of Windows.
- On Linux, an absolute `XDG_DATA_HOME` is the base. A value that is empty or relative counts as no value, as the specification of XDG tells. The base is then `$HOME/.local/share`.
- On Linux with no absolute `XDG_DATA_HOME`, and on macOS, a `HOME` that is not there, empty or relative gives `nullptr`.
- On Linux and macOS, the call makes no folder.

The sources of the base folders:

- Microsoft, `SHGetKnownFolderPath`: the caller frees the path "by calling CoTaskMemFree, whether SHGetKnownFolderPath succeeds or not." https://learn.microsoft.com/en-us/windows/win32/api/shlobj_core/nf-shlobj_core-shgetknownfolderpath
- Microsoft, `SHGetKnownFolderPath`: "The returned path does not include a trailing backslash."
- Microsoft, `KNOWNFOLDERID`: `FOLDERID_SavedGames` has the GUID `{4C5C32FF-BB9D-43b0-B5B4-2D72E54EAAA4}` and the default path "%USERPROFILE%\Saved Games". https://learn.microsoft.com/en-us/windows/win32/shell/knownfolderid
- Microsoft, `KNOWNFOLDERID`: `FOLDERID_LocalAppData` has the GUID `{F1B32785-6FBA-4FCF-9D55-7B8E7F157091}` and the default path "%LOCALAPPDATA% (%USERPROFILE%\AppData\Local)".
- Microsoft, `KF_FLAG_CREATE`: "The security provisions predefined for that folder are applied." https://learn.microsoft.com/en-us/windows/win32/api/shlobj_core/ne-shlobj_core-known_folder_flag
- Microsoft, `KF_FLAG_DONT_VERIFY`: without it, a folder that is "absent" gives "a failure code, and no path is returned."
- Microsoft, `KF_FLAG_DEFAULT_PATH`: "If this flag isn't set, then the function retrieves the current" and "possibly redirected" path "of the folder."
- Microsoft, `CoTaskMemFree`: "If this parameter is NULL, the function has no effect." Its DLL is `Ole32.dll`. https://learn.microsoft.com/en-us/windows/win32/api/combaseapi/nf-combaseapi-cotaskmemfree
- freedesktop.org, XDG Base Directory Specification, version 0.8: "If $XDG_DATA_HOME is either not set or empty, a default equal to $HOME/.local/share should be used." https://specifications.freedesktop.org/basedir/latest/
- freedesktop.org, the same page: "If an implementation encounters a relative path in any of these variables it should consider the path invalid and ignore it."
- Apple, File System Programming Guide: "Use this directory to store all app data files except those associated with the user's documents." https://developer.apple.com/library/archive/documentation/FileManagement/Conceptual/FileSystemProgrammingGuide/FileSystemOverview/FileSystemOverview.html
- Apple, the same page: `Library` "is the app's sandbox directory or the current user's home directory (if the app is not in a sandbox)."

## The output

`os_output_write` writes all the bytes to the standard output, with no change and no text mode. Thus a line end stays `\n`, also on Windows.

- On Linux and macOS, `write` runs until each byte is out. If a signal stops a write, the call writes the rest.
- On Windows, `WriteFile` writes the bytes to a pipe or to a file.
- A console of Windows gets `WriteConsoleW`, after the turn of the bytes from UTF-8 into UTF-16. Thus a path in UTF-8 shows right.
- The bytes of one call go out with one call of the system when the system takes them all at once.

It gives 0, or -1 if a write fails, for example when the output is closed. A write that fails does not end the caller.

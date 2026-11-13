# include/os.h

This header gives the calls of the operating system, in the same form on each system.
That is address space, shared libraries, threads, the clock, a mutex, an event, and folders, watches, paths, locks and signals.
It also gives processes, files, the environment, the output, a free port and the count of the processors.

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

## Functions

Each function that can fail is `[[nodiscard]]`: the caller must use its result.

| Function | Result |
| --- | --- |
| `os_reserve` | Reserves `reserve` bytes of address space, and makes the first `commit` bytes readable, writable and zero. The rest faults when touched, which a caller can use as a guard after a block. Gives `nullptr` on failure. |
| `os_commit` | Makes the first `bytes` of a reservation readable and writable. Thus a caller grows its memory, up to the reservation. Gives 0 on success. |
| `os_release` | Gives back all of a reservation. Each caller passes a base that `os_reserve` gave. |
| `os_lib_open` | Opens a shared library, or gives `nullptr`. For a path, the loader looks there for the library and for what it needs, so a library can bring its runtime beside itself. For a bare name, the loader uses the search of the operating system. |
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
| `os_folder_read_entry` | Gives the name of the next entry of the folder, and sets `*is_folder` to true if the entry is a folder. It skips `.` and `..`. The name stays valid until the next call. Gives `nullptr` after the last entry. |
| `os_folder_read_file` | Reads the whole file `name` of the folder. It gives the bytes, then one zero byte, and sets `*len` to the count of the bytes. The bytes belong to the folder: they stay valid until its next read or its close, and the caller does not free them. `name` is one name of the folder, with no separator. Gives `nullptr` if the file does not open or read, or if it is a folder. |
| `os_folder_entry_size` | Gives 0, and sets `*size` to the size in bytes of the file `name`, as *The folders* tells. It reads no byte of the file. `name` is one name of the folder, with no separator. Gives -1 and keeps `*size` if no file of `name` is there, if it is a folder, or if the call fails. |
| `os_folder_close` | Closes a folder, and frees its bytes. A null `f` does nothing. |
| `os_watch_start` | Starts a watch of the entries of `folder`, down to `depth` levels of folders, as *The watch* tells. Gives `nullptr` if `depth` is 0, if no folder of `folder` opens, or if a call that it needs fails. |
| `os_watch_take` | Gives the next change of a watch, and does not wait. It gives 1 and sets `*name`, or it gives 0, 2 or -1, as *The watch* tells. |
| `os_watch_stop` | Stops a watch, and frees it. A null `w` does nothing. |
| `os_path_whole` | Gives the absolute path of `path`, which the caller frees, or `nullptr`. |
| `os_lock` | Takes an exclusive lock on the file `path`, so that one process at a time holds it. It does not wait: it gives 0 on success, 1 if a lock holds the file already, and -1 if the file does not open. The mode is fixed, and the layer has one implementation of it. |
| `os_signal_catch` | Catches the signals that stop the process, and writes `line` at the first signal, as *The signals* tells. Gives 0 on success. |
| `os_signal_is_caught` | Tells if a signal set the flag of `os_signal_catch`. A caller reads it between two parts of its work, and then ends its work. |
| `os_process_start` | Starts a program with the fields of `start`, as *The processes* tells. Gives `nullptr` if the program does not start. |
| `os_process_write` | Writes all the `len` bytes into the input of the process. Gives 0 on success, and -1 if the input is closed or the write fails. |
| `os_process_close` | Closes one pipe of the process: `OS_INPUT`, `OS_OUTPUT` or `OS_ERRORS`. |
| `os_process_read` | Gives the next bytes of the output or of the errors, and waits up to `nanos` nanoseconds for them, as *The processes* tells. |
| `os_process_signal` | Sends `OS_INTERRUPT`, `OS_TERMINATION` or `OS_HANG_UP` to the process, as *A signal to a process* tells. Gives 0 on success. |
| `os_process_kill` | Ends the process at once, as *The processes* tells. |
| `os_process_wait` | Waits up to `nanos` nanoseconds for the end of the process, and gives its status, as *The processes* tells. |
| `os_process_free` | Frees a process. It kills a process that still runs, and waits for its end. A null `p` does nothing. |
| `os_file_write` | Writes all the bytes of the file `path`, as *The files* tells. Gives 0 on success. |
| `os_path_entry` | Tells what is at `path`, as *The files* tells. |
| `os_path_move` | Gives the file or the folder `from` the path `to`, as *The files* tells. Gives 0 on success. |
| `os_path_remove` | Removes what is at `path`, as *The files* tells. |
| `os_link_make` | Makes `path` a symbolic link to `target`, as *The files* tells. Gives 0 on success. |
| `os_env_get` | Gives a copy of the value of the variable `name`, as *The environment* tells. |
| `os_path_find` | Finds the program `name`, as *The environment* tells. |
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

A library loads by its path, so `os_lib_open` keeps the limit of the loader of each system. That limit is 4096 bytes on Linux, 1024 on macOS, and 260 characters on Windows.

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
- If the process still runs, the layer ends it with the status 3221225794, the status that Windows gives when the close ends it. Thus a hang-up ends the process on each version.
- On Windows, a signal reaches each process on that console: the process, and each process that it started there.
- The console drops what is written to it, because the process writes into its pipes.

Each system gives -1 for a signal to a process that started without `signals`.
Each system also gives -1 for a signal after a hang-up, because on Windows the hang-up ends the console.

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

- A new file gets the mode 0777 without the bits of the umask with `executable`, and 0666 without them else.
- A file that was there keeps its read bits and its write bits. With `executable`, it gets an execute bit for each read bit that it has. Else it loses each execute bit.
- Thus no call reads the umask: POSIX has no call that reads it with no change. A private file, for example of mode 0600, stays private.
- On Windows, a file has no execute bit, and `executable` changes nothing.

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

`os_path_remove` removes the file, the link or the folder at `path`, with all the entries of the folder:

- It removes a read-only entry too. It removes a link, and keeps its target.
- It gives 0 when it removed `path`, 1 when nothing was at `path`, and -1 on a failure.
- After a failure, some entries can be gone.

`os_link_make` makes `path` a symbolic link to `target`. A relative `target` starts in the folder of `path`.
An empty `target` gives -1 on each system, because a link to no path is of no use.
On Windows, a link needs the Developer Mode or the rights of an administrator.
On Windows, the link is a link to a folder if `target` is a folder at the call.

## The environment

`os_env_get` gives a copy of the value of the variable `name`, which the caller frees.
It gives `nullptr` if the variable is not there, or if the copy fails.

`os_path_find` gives the absolute path of the program `name`, which the caller frees, or `nullptr`:

- A name with a separator is a path. The call gives it if it is a file with an execute bit, or on Windows a file.
- Else the call looks in each folder of the `PATH` of the caller, in order.
- On Windows, if `name` has no extension, it tries `name` with each extension of `PATHEXT`, in order, and then `name` alone. With no `PATHEXT`, it tries no extension.
- It looks in the current folder only if `PATH` names it.

## The output

`os_output_write` writes all the bytes to the standard output, with no change and no text mode. Thus a line end stays `\n`, also on Windows.

- On Linux and macOS, `write` runs until each byte is out. If a signal stops a write, the call writes the rest.
- On Windows, `WriteFile` writes the bytes to a pipe or to a file.
- A console of Windows gets `WriteConsoleW`, after the turn of the bytes from UTF-8 into UTF-16. Thus a path in UTF-8 shows right.
- The bytes of one call go out with one call of the system when the system takes them all at once.

It gives 0, or -1 if a write fails, for example when the output is closed. A write that fails does not end the caller.

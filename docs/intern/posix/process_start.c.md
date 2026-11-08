# src/posix/process_start.c

This file starts a process on Linux and macOS.
Each function keeps the contract of `docs/extern/os.h.md`, *The processes*.

## The start

1. `find_program` gives the path of the program: a copy of `argv[0]` if it holds a `/`, else `os_path_find`.
2. `os_build_env` of `src/posix/process_env.c` makes the environment of the process.
3. `open_pipes` opens the pipes: input, output, errors if `errors_apart`, a gate and a report.
4. `pthread_sigmask` blocks each signal, so no handler of the caller runs in the child.
5. `fork` makes the child, which runs `os_run_child` of `src/posix/process_child.c`.
6. The parent gives back its mask, and closes the ends of the child.
7. `os_watch_exit` starts the watch of the end of the process.
8. The parent closes the gate, so the child goes on to `execve`.
9. `read` of the report gives no byte if `execve` passed. A byte tells that it failed.

After a failed `execve`, `waitpid` takes the child, and the start gives `nullptr`.
The ends of the parent get `O_NONBLOCK`, so a read and a write can wait for the wake pipe too.

## The gate

The child waits on the gate until the parent closes it. Thus the watch of the end exists before the child can end.
On macOS, a watch of a process that ended already fails. The gate makes that case impossible.

## Limits

On macOS, `pipe` and `fcntl` set `FD_CLOEXEC` in 2 calls. A `fork` of another thread between them gives its child a copy of the end.
Then that child keeps the pipe open, and a read of the output can wait for the end of that child too.

## Functions

| Function | Result |
| --- | --- |
| `find_program(name)` | Gives the path of the program, which the caller frees, or `nullptr`. |
| `open_pipes(p, start, child)` | Opens the pipes of *The start*. Gives 0, or -1. |
| `start_child(p, start, child)` | Does steps 4 to 9 of *The start*. Gives 0 if `execve` passed, else -1. |
| `close_all(p, child)` | Closes each end that is open, and frees the parts of the process. |
| `os_process_start(start)` | Starts the process as *The start* tells. Gives the process, or `nullptr`. |

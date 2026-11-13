# src/posix/process_start.c

This file starts a process on Linux and macOS.
Each function keeps the contract of `docs/extern/os.h.md`, *The processes*.

## The start

1. `os_find_program` gives the path of the program, and `os_build_env` the environment: `src/posix/process_env.c`.
2. `open_pipes` opens the pipes of `os_launch_t`: input, output, errors if `errors_apart`, a gate and a report. Then the wake pipe.
3. `pthread_sigmask` blocks each signal, so no handler of the caller runs in the child.
4. `fork` makes the child, which runs `os_run_child` of `src/posix/process_child.c`.
5. The parent gives back its mask, and closes the ends of the child, except the end of the gate.
6. `os_watch_exit` starts the watch of the end of the process. If it fails, `kill` ends the child.
7. Else the parent writes 1 byte into the gate, so the child goes on to `execve`. Then the parent closes both ends of the gate.
8. `read` of the report gives no byte if `execve` passed. A byte tells that it failed.

After a failure, `waitid` takes the child, and the start gives `nullptr`.
Else `keep_ends` keeps the ends of the parent, and gives them `O_NONBLOCK`, so a read and a write can wait for the wake pipe too.
It also gives the input to `os_quiet_pipe`, so a broken write gives no signal on macOS.
`fcntl` and `dup2` on an open descriptor fail only for a bad descriptor, so the file does not ask their result.

## The gate

The child waits on the gate for 1 byte, or for its end. Thus the watch of the end exists before the child can end.
On macOS, a watch of a process that ended already fails. The gate makes that case impossible.

The byte opens the gate also when another child holds a copy of its write end.
2 threads can start processes at one time. Then the child of each `fork` gets the open pipes of the other start, until its `execve`.
If each child waited for the end of the gate, each one would hold the gate of the other, and both would wait forever.

The parent keeps the read end of the gate until after the write. Thus the write gives no `SIGPIPE` when the child ended already.

## Limits

On macOS, `pipe` and `fcntl` set `FD_CLOEXEC` in 2 calls. A `fork` of another thread between them gives its child a copy of the end.
Then that child keeps the pipe open, and a read of the output can wait for the end of that child too.

A handler of a signal of the caller that has no `SA_RESTART` can cut the read of the report: `os_signal_catch` sets `SA_RESTART`.
After such a cut, a failed `execve` looks like a start: the read gives 0, and the wait gives the status 127.

## Functions

| Function | Result |
| --- | --- |
| `open_pipes(p, start, pipes)` | Opens the pipes of *The start*. Gives 0, or -1. |
| `keep_ends(p, start, pipes)` | Keeps the ends of the parent in the process, as *The start* tells. |
| `finish_start(p, start, pipes)` | Does steps 6 to 8 of *The start*. Gives 0 if `execve` passed, else -1. |
| `start_child(p, start, launch)` | Does steps 3 to 5 of *The start*, then `finish_start`. Gives 0, or -1. |
| `os_process_start(start)` | Starts the process as *The start* tells. Gives the process, or `nullptr`. |

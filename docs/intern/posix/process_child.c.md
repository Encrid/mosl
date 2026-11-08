# src/posix/process_child.c

This file holds the steps of the child of `os_process_start`, between `fork` and `execve`, on Linux and macOS.
It also closes the pipes of a start, and drops a process that did not start.

## The child

After `fork` in a program with threads, the child can call only the functions that POSIX names safe after `fork`.
So the parent makes the path, the arguments and the environment before `fork`, and the child makes no allocation.

1. `sigaction` gives `SIGINT`, `SIGTERM`, `SIGHUP` and `SIGPIPE` their default action. With `ignore_interrupt`, `SIGINT` is ignored.
2. `read` of the gate waits until the parent closes it.
3. `dup2` puts the input on 0, the output on 1, and the errors on 2. Without `errors_apart`, the output goes on 2 too.
4. `chdir` goes into `folder`, if the start gives one.
5. `sigprocmask` unblocks each signal, and `execve` starts the program.
6. If a step fails, `fail_child` writes `errno` into the report, and ends with `_exit` and 127.

The child does not ask the result of `dup2`, for the reason of `docs/intern/posix/process_start.c.md`, *The start*.

## Functions

| Function | Result |
| --- | --- |
| `set_signals(ignore)` | Gives the 4 signals their action at the start, as step 1 tells. |
| `fail_child(report)` | Writes `errno` into the report, and ends the child. |
| `os_run_child(start, launch)` | Does the steps of *The child*. It does not return. |
| `os_close_pipes(pipes, count)` | Closes each end of the `count` pipes that is open, and sets it to -1. |
| `os_drop_process(p)` | Closes the wake pipe and the watch of the end, and frees the process. |

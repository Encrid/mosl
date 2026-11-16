# src/posix/process_child.c

This file holds the steps of the child of `os_process_start`, between `fork` and `execve`, on Linux and macOS.

## The child

After `fork` in a program with threads, the child can call only the functions that POSIX names safe after `fork`.
So the parent makes the path, the arguments and the environment before `fork`, and the child makes no allocation.

1. `sigaction` gives `SIGINT`, `SIGTERM`, `SIGHUP` and `SIGPIPE` their default action. With `ignore_interrupt`, `SIGINT` is ignored.
2. `read` of the gate waits for the byte of the parent. If the parent ended, the end of the gate stops the wait.
3. `put_end` puts the input on 0, the output on 1, and the errors on 2. Without `errors_apart`, the output goes on 2 too.
4. `chdir` goes into `folder`, if the start gives one.
5. `sigprocmask` unblocks each signal, and `execve` starts the program.
6. If a step fails, `fail_child` writes `errno` into the report, and ends with `_exit` and 127.

The child does not ask the result of `fcntl` and of `dup2`, for the reason of `docs/intern/posix/process_start.c.md`, *The start*.

## An end that is a stream already

Each end of a pipe has `FD_CLOEXEC`, so `execve` closes each end that the program must not get.
`dup2` gives the stream a copy of the end with no `FD_CLOEXEC`.

A caller can have no descriptor 0. The input pipe opens first, so its read end is then the descriptor 0.
`dup2` of a descriptor onto itself changes nothing. The end would keep `FD_CLOEXEC`, and the program would get no input.
So `put_end` removes `FD_CLOEXEC` with `fcntl` when the end is the stream already.

A pipe takes the 2 lowest descriptors that are free, and the input pipe opens first.
Thus each other end that the child needs is 3 or more.
No `dup2` of step 3 writes over an end that a later one needs.

## Functions

| Function | Result |
| --- | --- |
| `set_signals(ignore)` | Gives the 4 signals their action at the start, as step 1 tells. |
| `fail_child(report)` | Writes `errno` into the report, and ends the child. |
| `put_end(end, stream)` | Makes `end` the stream of the program, as *An end that is a stream already* tells. |
| `os_run_child(start, launch)` | Does the steps of *The child*. It does not return. |

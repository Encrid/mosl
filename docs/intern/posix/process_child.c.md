# src/posix/process_child.c

This file holds the steps of the child of `os_process_start`, between `fork` and `execve`, on Linux and macOS.

## The child

After `fork` in a program with threads, the child can call only the functions that POSIX names safe after `fork`.
So the parent makes the path, the arguments and the environment before `fork`, and the child makes no allocation.

1. `sigaction` gives `SIGINT`, `SIGTERM`, `SIGHUP` and `SIGPIPE` their default action. With `ignore_interrupt`, `SIGINT` is ignored.
2. `read` of the gate waits until the parent closes it.
3. `dup2` puts the input on 0, the output on 1, and the errors on 2.
4. Without `errors_apart`, the output goes on 2 too.
5. `chdir` goes into `folder`, if the start gives one.
6. `sigprocmask` unblocks each signal, and `execve` starts the program.
7. If a step fails, the child writes `errno` into the report, and ends with `_exit` and 127.

## Functions

| Function | Result |
| --- | --- |
| `set_signals(ignore)` | Gives the 4 signals their action at the start, as step 1 tells. |
| `fail_child(report)` | Writes `errno` into the report, and ends the child. |
| `os_run_child(p, start, child)` | Does the steps of *The child*. It does not return. |

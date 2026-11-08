# src/linux/pipe_signal.c

On Linux, this file takes the `SIGPIPE` that a broken write leaves: `docs/intern/posix/write_all.c.md`, *The writes*.

## The signal

Linux makes a blocked signal pending also when the program ignores it. Thus after a write that ends with `EPIPE`, the signal always waits.
`os_take_pipe_signal` calls `sigwait`, which returns at once and takes the signal.

## Functions

| Function | Result |
| --- | --- |
| `os_take_pipe_signal(pipe_only)` | Takes the waiting `SIGPIPE` with `sigwait`. |

# src/macos/pipe_signal.c

This file gives `os_take_pipe_signal` for macOS, which the writes of `src/posix/write_all.c` call.

## The signal

macOS drops a signal that the program ignores, also while the signal is blocked.
So a broken write leaves a waiting signal only when the program does not ignore it.
`os_take_pipe_signal` asks `os_is_pipe_waiting` first. A `sigwait` with no waiting signal would wait for ever.

## Functions

| Function | Result |
| --- | --- |
| `os_take_pipe_signal(pipe_only)` | Takes `SIGPIPE` with `sigwait` if it waits, else does nothing. |

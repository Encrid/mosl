# src/posix/process_end.c

This file signals, kills, waits for and frees a process on Linux and macOS.
Each function keeps the contract of `docs/extern/os.h.md`, *The processes*, and `docs/extern/os.h.md`, *A signal to a process*.

## The lock

The system can give the number of a process that ended to a new process, after `waitpid` takes the old one.
So `kill` and `waitpid` take the mutex of the process. `kill` runs only while the process is not taken yet.
A wait does not hold the mutex while it waits. Thus a kill from another thread does not wait for the wait.

## The end

- `os_process_kill` writes a byte into the wake pipe, then sends `SIGKILL`. A read or a write that waits sees the byte first.
- `os_process_wait` asks `os_wait_exit` for the end, then `waitpid` takes the process with `WNOHANG`.
- A process that a signal ended gets minus the number of the signal as its status.
- `os_process_signal` sends `SIGINT`, `SIGTERM` or `SIGHUP`. After `SIGHUP`, each signal gives -1.
- `os_process_free` kills a process that runs, waits for its end, and closes each end.

## Functions

| Function | Result |
| --- | --- |
| `send_signal(p, number)` | Sends `number` with `kill` under the mutex, if the process is not taken. Gives 0, or -1. |
| `os_process_signal(p, which)` | Sends the signal of `which`, as *The end* tells. Gives 0, or -1. |
| `os_process_kill(p)` | Wakes the waits, and sends `SIGKILL`. |
| `os_process_wait(p, nanos, status)` | Waits as *The end* tells. Gives 0, 1 or -1. |
| `os_process_free(p)` | Frees the process as *The end* tells. A null `p` does nothing. |

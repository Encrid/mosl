# src/windows/process_end.c

On Windows, this file kills a process, waits for its end, and frees it.
Each function keeps the contract of `docs/extern/os.h.md`, *The processes*.

## The end

- `os_process_kill` sets the stop event, which ends each read and each write that waits. Then `TerminateProcess` ends the process with the code 1.
- `os_process_wait` waits with `WaitForSingleObject` up to the time that is left, and `GetExitCodeProcess` gives the status.
- `os_process_free` kills a process that runs, and waits for its end. Then `os_drop_process` closes the streams, the console and each handle.
- A start that fails calls `os_drop_process` too, so `test --leaks` does not count a free of a process that never started.

## The time

`os_get_wait` gives the milliseconds until a deadline, rounded up, for `WaitForSingleObject` and `WaitForMultipleObjects`.
`OS_FOREVER` gives `INFINITE`. A deadline that passed gives 0. A wait of more than about 49 days gives the longest finite wait.

## Functions

| Function | Result |
| --- | --- |
| `os_get_wait(deadline)` | Gives the milliseconds of *The time*. |
| `os_process_kill(p)` | Stops the waits, and ends the process. |
| `os_process_wait(p, nanos, status)` | Waits as *The end* tells. Gives 0, 1 or -1. |
| `os_drop_process(p)` | Closes the streams, the console and each handle that is open, and frees the process. |
| `os_process_free(p)` | Frees the process as *The end* tells. A null `p` does nothing. |

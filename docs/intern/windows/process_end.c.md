# src/windows/process_end.c

On Windows, this file holds the kill, the wait and the free of a process, as stand-ins: `docs/intern/windows/process.c.md`.

## Functions

| Function | Result |
| --- | --- |
| `os_process_kill(p)` | Does nothing. |
| `os_process_wait(p, nanos, status)` | Gives -1. |
| `os_process_free(p)` | Does nothing. |

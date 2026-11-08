# src/windows/process.c

On Windows, this file holds the start, the pipes and the signals of a process. Its calls refuse each request until the code of Windows comes.

## Functions

| Function | Result |
| --- | --- |
| `os_process_start(start)` | Gives `nullptr`. |
| `os_process_write(p, bytes, len)` | Gives -1. |
| `os_process_close(p, stream)` | Does nothing. |
| `os_process_read(p, nanos, bytes, len)` | Gives -1. |
| `os_process_signal(p, which)` | Gives -1. |

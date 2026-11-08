# src/windows/process_read.c

On Windows, `os_process_read` waits for the overlapped reads of the output and of the errors.
Each function keeps the contract of `docs/extern/os.h.md`, *The processes*.

## The read

1. `start_read` starts an overlapped `ReadFile` on each open pipe that has no read yet. The end of a pipe closes it.
2. `WaitForMultipleObjects` waits for the stop event and for the events of the reads, up to the time that is left.
3. The stop event gives 0: `os_process_kill` set it.
4. A read that ended gives its bytes, which stay in the buffer of the pipe. Bytes give `OS_OUTPUT` or `OS_ERRORS`.
5. With both pipes closed, the result is 0. After the time, it is -2.

A read that waits stays after the call. The next call waits for it, so no byte is lost.
The first pipe that a read asks changes at each read, as on Linux and macOS: `docs/intern/posix/process_read.c.md`, *The read*.

## Functions

| Function | Result |
| --- | --- |
| `start_read(s)` | Does step 1 for the stream `s`. Gives 0, or -1. |
| `take_bytes(p, index, len)` | Gives `OS_OUTPUT` or `OS_ERRORS` for the bytes of the stream `index`, 1 after its end, or -1. |
| `queue_reads(p, waits, indexes)` | Starts the reads, and puts their events into `waits`. Gives the count of the waits, or -1. |
| `read_once(p, deadline, len)` | Does steps 1 to 4 one time. Gives 1 when the read must wait again. |
| `os_process_read(p, nanos, bytes, len)` | Reads as *The read* tells. |

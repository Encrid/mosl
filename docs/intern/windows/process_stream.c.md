# src/windows/process_stream.c

On Windows, `os_close_stream` closes the output or the errors of a process, also while a read waits on it.

## The close

A read can wait on the pipe. `CancelIoEx` ends it, and `GetOverlappedResult` waits until the system no longer writes into the buffer.
Then `CloseHandle` closes the pipe. A stream with no pipe does nothing.

## Functions

| Function | Result |
| --- | --- |
| `os_close_stream(s)` | Closes the stream `s`, as *The close* tells. |

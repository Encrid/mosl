# src/windows/process_pipes.c

On Windows, this file opens the pipes of a process, writes into its input, and closes its pipes.
Each function keeps the contract of `docs/extern/os.h.md`, *The processes*.

## The pipes

An anonymous pipe has no wait with a time limit. So each pipe is a named pipe, with a name of the process and of the place of the pipe.
The end of the parent has `FILE_FLAG_OVERLAPPED`. Thus each read and each write waits for its event and for the stop of the process at once.
The end of the child is synchronous and can be inherited, as the child expects. Each pipe holds 65536 bytes.

## The write

`write_part` writes at most 1 MiB with an overlapped `WriteFile`, then waits for the end of the write or for the stop event.
After the stop, `CancelIoEx` ends the write, and the write gives -1. A write into a pipe that the child closed gives -1 too.

## Functions

| Function | Result |
| --- | --- |
| `open_server(name, inbound)` | Makes the end of the parent of the named pipe `name`. Gives it, or `nullptr`. |
| `os_open_pipe(p, index, inbound, ends)` | Opens the pipe `index` of *The pipes*. `ends[0]` is the end of the parent, `ends[1]` the end of the child. Gives 0, or -1. |
| `write_part(p, at, part, wrote)` | Writes a part as *The write* tells. Gives 0, or -1. |
| `os_process_write(p, bytes, len)` | Writes all the bytes in parts. Gives 0, or -1. |
| `os_process_close(p, stream)` | Closes the pipe `stream`. |

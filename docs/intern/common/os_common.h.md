# src/common/os_common.h

This header declares the parts of `src/common` for the files of each system.
It includes `<stdint.h>` and `<stdio.h>`.

## Constants

`OS_AGAIN` is 3: the result of a step of a take that has no result yet.
The take of a watch does steps while a step gives `OS_AGAIN` and the watch did not stop. Then it gives the last result, or -1 after a stop.
No result of `os_watch_take` is 3.

## Functions

| Function | Result |
| --- | --- |
| `os_read_stream(file, size, kept, len)` | Reads a stream whole into the buffer of a folder: `docs/intern/common/read_stream.c.md`, *The buffer*. |
| `os_set_loopback(address)` | Writes the address of the loopback: `docs/intern/common/port_bytes.c.md`, *The order*. |
| `os_get_port(port)` | Reads a port: `docs/intern/common/port_bytes.c.md`, *The order*. |
| `os_is_within(name, depth)` | Tells if a name of a watch is within its depth: `docs/intern/common/watch_depth.c.md`, *The depth*. |

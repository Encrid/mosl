# src/posix/port.c

This file finds a free port of TCP on Linux and macOS.

## The port

`os_get_free_port` makes a socket of TCP, and binds it to `127.0.0.1` with the port 0, so the system selects a free port.
`getsockname` reads that port, and `close` frees the socket. Each failure gives -1.

## Functions

| Function | Result |
| --- | --- |
| `os_get_free_port()` | Gives the port that the system selected, or -1. |

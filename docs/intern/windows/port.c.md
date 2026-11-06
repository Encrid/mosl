# src/windows/port.c

On Windows, this file loads the library of sockets for `os_get_free_port`.

## The library

A program that links `os.a` links no library of sockets, as the table *Functions* of `docs/extern/os.h.md` tells.
So `os_get_free_port` loads `ws2_32.dll` with `LoadLibraryW`, and finds each call with `GetProcAddress`, in the order of `NAMES`.
`src/windows/os_sockets.h` gives the types of the calls, and `sockets_t`, which holds them.
It calls `WSAStartup`, and at the end `WSACleanup` and `FreeLibrary`.

## The port

The rest follows `docs/intern/posix/port.c.md`, *The port*, with `closesocket` in place of `close`.

## Functions

| Function | Result |
| --- | --- |
| `load_calls(library, calls)` | Finds each call of `NAMES` in `library`. Gives false if one is not there. |
| `get_port(calls)` | Binds a socket with the calls, and gives its port, or -1. |
| `os_get_free_port()` | Gives the port of `getsockname`, or -1. |

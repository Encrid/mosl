# src/windows/port.c

On Windows, this file loads the library of sockets for `os_get_free_port`.

## The library

A program that links `os.a` links no library of sockets, as the table *Functions* of `docs/extern/os.h.md` tells.
So `os_get_free_port` loads `ws2_32.dll` with `LoadLibraryW`, and finds each call with `GetProcAddress`.
It calls `WSAStartup`, and at the end `WSACleanup` and `FreeLibrary`.

## The port

The rest follows `docs/intern/posix/port.c.md`, *The port*, with `closesocket` in place of `close`.

## Functions

| Function | Result |
| --- | --- |
| `os_get_free_port()` | Gives the port of `getsockname`, or -1. |

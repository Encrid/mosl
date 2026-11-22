# src/posix/library.c

This file gives the shared libraries of `include/os.h` on Linux and macOS: a library opened, looked into, pinned and closed.
Each function keeps the contract of `docs/extern/os.h.md`, *Functions*.

## Opening

`os_lib_open` calls `dlopen` with `RTLD_NOW` and `RTLD_LOCAL`.
`RTLD_NOW` resolves every symbol of the library before `dlopen` returns.
`RTLD_LOCAL` keeps the names of the library to itself: they do not resolve the symbols of a library that opens later.
The loader opens a path as it is, and searches for a bare name, with no flag of this file.
The loader looks beside the library for what it needs only if the library asks for it: `docs/extern/os.h.md`, *The libraries*.

## Pinning

`os_lib_pin` calls `dlopen` with `RTLD_NOLOAD` and `RTLD_NODELETE`.
`RTLD_NOLOAD` opens nothing: `dlopen` gives the library only if it is already open, else `nullptr`.
On Linux, `RTLD_NODELETE` stays on the library, so no later `dlclose` unloads it.
On macOS, the flag did not keep a library after its last `dlclose`. So `os_lib_pin` never closes the handle that its `dlopen` gives.

Each pin keeps its own handle. Thus the file needs no table and no lock, and the count of pins has no limit.
A second pin of a library keeps one more handle of a library that stays anyway.

## Closing

`os_lib_close` calls `dlclose`.

## Functions

| Function | Result |
| --- | --- |
| `os_lib_open` | Opens the library with `dlopen`. Gives `nullptr` on failure. |
| `os_lib_get_symbol` | Gives what `dlsym` finds of `name` in the library, or `nullptr`. |
| `os_lib_close` | Closes the library with `dlclose`. |
| `os_lib_pin` | Pins the open library of `path`, as *Pinning* tells. Gives 0, or -1 if `dlopen` finds no such library. |

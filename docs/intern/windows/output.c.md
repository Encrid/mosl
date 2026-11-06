# src/windows/output.c

On Windows, this file gives `os_output_write` for a console and for each other handle.
Each function keeps the contract of `docs/extern/os.h.md`, *The output*.

## The write

`GetStdHandle` gives the handle of the standard output. With no handle, the call gives -1.
`GetConsoleMode` tells if the handle is a console:

- A console gets the bytes in UTF-16. `MultiByteToWideChar` turns the `len` bytes, and `WriteConsoleW` writes the characters until each one is out.
- Each other handle gets the bytes with no change, through `os_write_all` of `src/windows/write_all.c`.

A character of UTF-8 that one call cuts in 2 becomes 2 characters of replacement in a console. So a caller writes whole characters in each call.

## Functions

| Function | Result |
| --- | --- |
| `os_output_write(bytes, len)` | Writes the bytes into the console or into the handle, as *The write* tells. Gives 0, or -1. |

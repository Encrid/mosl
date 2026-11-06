# src/windows/write_all.c

This file writes each byte into a handle of Windows.

## The writes

`WriteFile` takes a count of 32 bits. So `os_write_all` writes at most 1 GiB in each call, until each byte is out.
A call that fails, or that writes no byte, gives -1.

## Functions

| Function | Result |
| --- | --- |
| `os_write_all(handle, bytes, len)` | Gives 0 when `WriteFile` wrote the `len` bytes into `handle`, else -1. |

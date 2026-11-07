# src/windows/write_all.c

This file writes each byte into a handle of Windows.

## The writes

`WriteFile` takes a count of 32 bits. So `os_write_all` writes at most 1 MiB in each call, until each byte is out.
A call of 1 MiB costs no more time than a larger call, and a case of 2 MiB runs each branch.
A call that fails gives -1. For a handle with no overlap, `WriteFile` writes at least 1 byte when it passes.

## Functions

| Function | Result |
| --- | --- |
| `os_write_all(handle, bytes, len)` | Gives 0 when `WriteFile` wrote the `len` bytes into `handle`, else -1. |

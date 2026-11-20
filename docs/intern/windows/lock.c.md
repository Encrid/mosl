# src/windows/lock.c

On Windows, `os_lock` locks the first byte of a file, and `os_unlock` unlocks it.
Each function keeps the contract of `docs/extern/os.h.md`, *The locks*.

## The lock

The steps are those of `docs/intern/posix/lock.c.md`, *The lock*, with the calls of the C library of Windows:

| Step | Call |
| --- | --- |
| Open | `_open`, which creates the file if it is not there. |
| Lock | `_locking` with `_LK_NBLCK` on 1 byte. |
| Close | `_close`. |
| Unlock | `_locking` with `_LK_UNLCK` on the same byte. |

A lock belongs to the handle of the file. Thus a second `_open` in the same process cannot lock the byte while the first holds it.
No call reads or writes the file, so its position stays at the first byte for the unlock.

## Functions

| Function | Result |
| --- | --- |
| `os_lock(path, lock)` | Locks the byte as *The lock* tells. Gives 0, 1 or -1. |
| `os_unlock(lock)` | Unlocks the byte, closes the file and frees the handle. A null `lock` does nothing. |

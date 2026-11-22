# src/posix/lock.c

`os_lock` takes the lock of a file on Linux and macOS, and `os_unlock` ends it.
Each function keeps the contract of `docs/extern/os.h.md`, *The locks*.

## The lock

1. `os_lock` sets `*lock` to `nullptr`, and takes the memory of the handle.
2. `open` opens the path with `O_CREAT` and `O_CLOEXEC`. Thus a started process does not get the file.
3. `flock` asks for `LOCK_EX` with `LOCK_NB`, so the call does not wait.
4. If `flock` refuses, `close` closes the file. The result is 1 for `EWOULDBLOCK`, and -1 for each other error.
5. On each result that is not 0, the call frees the handle. On 0, `*lock` gets the handle.

The result is -1 when the memory or `open` fails.
`errno` is read before `close`, which can change it.
A second `open` of the file in the same process gives a different open file description. Thus `flock` refuses it while the first holds the lock.

`os_unlock` gives `LOCK_UN` to `flock`, closes the file and frees the handle.

## Functions

| Function | Result |
| --- | --- |
| `os_lock(path, lock)` | Takes the lock as *The lock* tells. Gives 0, 1 or -1. |
| `os_unlock(lock)` | Ends the lock. A null `lock` does nothing. |

# src/linux/pidfd.c

On Linux, this file opens the descriptor of a process with the system call `pidfd_open`.

## The call

glibc 2.31 has no function for `pidfd_open`, so the file calls `syscall`.
`syscall` takes a list of arguments of no fixed length, and `fail.py` cannot make such a call fail.
So the call sits in a function of its own, which a case can make fail by its name.
The targets of Linux need kernel 5.10 or newer, and `pidfd_open` came in kernel 5.3. So the call has no fallback.

## Functions

| Function | Result |
| --- | --- |
| `os_pidfd_open(pid)` | Gives the descriptor of the process `pid`, or -1. |

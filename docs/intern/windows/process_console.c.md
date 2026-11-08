# src/windows/process_console.c

On Windows, this file makes the pseudo console of a process, and sends its signals.
Each function keeps the contract of `docs/extern/os.h.md`, *A signal to a process*.

## The console

`os_make_console` makes a console of 80 columns and 25 lines with `CreatePseudoConsole`.
Its input is a pipe, which the parent writes into. Its output is the device `NUL`, which takes each byte at once.
Thus the console never waits for a reader, and `ClosePseudoConsole` never waits for one either, on each version of Windows.

## The signals

- The interrupt writes the byte 3 into the input of the console, which sends `CTRL_C_EVENT`.
- The termination writes the key Ctrl-Break in `win32-input-mode`: `ESC [ 3 ; 70 ; 0 ; 1 ; 8 ; 1 _`. That is `VK_CANCEL`, its scan code, and the Ctrl key.
- The hang-up calls `ClosePseudoConsole`, then `WaitForSingleObject` with a grace of 1 second, then `TerminateProcess` with 3221225794.
  The reason is in `docs/extern/os.h.md`, *A signal to a process*. On a process that ended, `TerminateProcess` does nothing, and the status stays.
  So the code has no choice, and each version of Windows runs the same steps.

## Functions

| Function | Result |
| --- | --- |
| `os_make_console(p)` | Makes the console of *The console*. Gives 0, or -1. |
| `write_key(p, key, len)` | Writes `key` into the input of the console. Gives 0, or -1. |
| `os_process_signal(p, which)` | Sends the signal `which`, as *The signals* tells. Gives 0, or -1. |

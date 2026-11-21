# The tests of the processes

This document gives the limits of the signal rows of `tests/shared/cases/process.toml` on Windows.
`docs/extern/os.h.md`, *A signal to a process*, gives the limits of Windows that these rows meet, with their sources.

## The child that tells its signal

The rows r14 to r17 start a child that writes the name of each signal that it gets, for example `got interrupt`.
Then the child ends with the status of that signal. Thus each claim names the signal, and a wrong signal makes the row fail.

The child writes `ready` at the end of its start. The test sends the signal only after that line.
Each read of the test waits at most 10 seconds. Thus a signal that the console loses makes only its own row fail.

## r14: an interrupt that the process ignores

The row r14 claims that with `ignore_interrupt`, the process ignores an interrupt that comes after its start.
The row cannot show if the process ignored the interrupt, or if the console lost it. The 2 give the same result.
The proof is indirect:

1. The child writes `ready` at the end of its start.
2. The test sends the interrupt after that line.
3. The test waits up to 0.5 seconds for the end of the process.
4. If the process still runs, the test sends the termination.

Thus the line `ready` came before the 2 keys, and the 2 keys come at least 0.5 seconds apart.
A loss before the end of the start cannot explain the result. A dispatch that holds the 2 keys cannot explain it.

## Causes of a flaky run

- The false variant f14 starts the process without `ignore_interrupt`. If the console loses its interrupt, the test sends the termination.
  Then the child writes `got termination`, the claim of r14 is true for f14, and the break run fails at f14.
- After the close, the hang-up ends the process with `TerminateProcess` at the end of a grace of 1 second. If the child does not write `got hang-up` in that time, the test run fails at r17.
  This failure shows, thus no row passes by error.

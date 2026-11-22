# The tests of the processes

This document gives the limits of the signal rows and of the kill rows of `tests/shared/cases/process.toml` on Windows.
`docs/extern/os.h.md`, *A signal to a process*, gives the limits of Windows that the signal rows meet, with their sources.

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

## r12: a kill that ends a read that waits

The row r12 claims that `os_process_kill` from another thread ends a read that waits. The thread of the read and the kill run in no fixed order.
Thus the test first makes a read wait, with no order of threads:

1. The child `hold` writes nothing.
2. Before the thread starts, the test reads with a limit of 1 ms. The read gives -2, and on Windows its read of the pipe stays pending.
3. The thread reads with no limit, and the test kills the process.

On Windows, `os_process_kill` sets the stop event before `TerminateProcess`, and the stop event stays set.
The read of the thread finds the pending read, so it waits for the stop event and for that read together.
If it waits before the kill, the stop event ends the wait. If it comes after the kill, the stop event is set already, and the wait gives the first set event, the stop event.
Thus each run reaches the end of a read by the stop event, in `src/windows/process_read.c`.

The source of the order of the wait:

- Microsoft, `WaitForMultipleObjects`: "If multiple objects become signaled, the function returns the index of the first handle in the array whose object was signaled." https://learn.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-waitformultipleobjects

## r13: a kill that ends a write that waits

The row r13 claims that `os_process_kill` from another thread ends a write that waits. The thread writes 4 MiB to the child `stall`, which reads nothing.
A write has no limit of time, so the test cannot make a write wait before the thread starts, as for r12.
Thus the test waits 100 ms after the start of the thread, and then kills the process.

On Windows, the stop event ends the wait of the write only if that wait starts before the teardown of the child closes its pipe.
If the teardown comes first, `WriteFile` fails, and the write gives -1 with no wait. The claim of the row holds both ways.
That order is outside the code: `docs/extern/os.h.md` gives no way to see the teardown of a killed process.
Thus the branch of the stop event in `src/windows/process_pipes.c` has an exemption with `may_reach` in `.toolchain/config/checks.toml`.
The wait of 100 ms lets the thread reach its wait first in nearly each run.

## Causes of a flaky run

- The false variant f14 starts the process without `ignore_interrupt`. If the console loses its interrupt, the test sends the termination.
  Then the child writes `got termination`, the claim of r14 is true for f14, and the break run fails at f14.
- After the close, the hang-up ends the process with `TerminateProcess` at the end of a grace of 1 second. If the child does not write `got hang-up` in that time, the test run fails at r17.
  This failure shows, thus no row passes by error.

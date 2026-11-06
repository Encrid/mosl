# src/posix/signal.c

On Linux and macOS, this file gives the signals of `include/os.h`: a flag for the caller, a line, and an end at once.
Each function keeps the contract of `docs/extern/os.h.md`, *Functions*.

## The state

`get_stop` keeps one `struct os_stop` of `src/posix/os_types.h`: the flag, the line of the caller and the length of the line.
A handler of a signal can reach only an object of static storage, and safely only an atomic object that has no lock.
So each field is atomic, and each of them has no lock on each target. `line` is an `atomic_uintptr_t` that holds the address of the line.
Each access to a field gives `memory_order_seq_cst`, so all the threads and the handler see the accesses in one order.
`os_signal_catch` stores the line and its length before it gives the handler to a signal.

## The handler

`os_signal_catch` gives `catch_stop` to `sigaction` for `SIGINT`, `SIGTERM` and `SIGHUP`, through `catch_one`.
`catch_stop` sets the flag with `atomic_exchange_explicit`, which also gives the value before the set.
At the first signal, `catch_stop` writes the line with `write` on `STDOUT_FILENO`.
The system reads the bytes of the line, and the handler reads only atomic fields.

If the flag was set already, the signal is a second one, and `catch_stop` calls `_Exit` with 1.
`_Exit` ends the process with no handler of `atexit` and no flush of a stream, so the end is at once.
`write` and `_Exit` are safe in a handler of a signal.

`catch_one` first reads the action of the signal. If it is `SIG_IGN`, the parent wants the signal ignored, and `catch_one` keeps it.
`SA_RESTART` makes a call that a signal interrupts start again. Thus a stop does not make a read or a write of the program fail.

The write of the line waits when the output is full.
`SA_NODEFER` lets a second signal of the same kind run the handler again in that time, so the second signal still ends the process.
An empty `sa_mask` blocks no other signal while the handler runs, because `atomic_exchange_explicit` already orders two signals.

## The broken pipe

`os_signal_catch` sets `SIGPIPE` to `SIG_IGN`. A write to a pipe that nobody reads then fails with `EPIPE`, and the process goes on.

## Functions

| Function | Result |
| --- | --- |
| `get_stop` | Gives the state of *The state*. |
| `catch_stop(number)` | The handler of *The handler*. It sets the flag and writes the line, or ends the process with status 1 if the flag was set. |
| `catch_one(number, act)` | Gives `act` to the signal `number`, if the signal is not ignored. Gives what `sigaction` gives, or 0 for an ignored signal. |
| `os_signal_catch(line)` | Stores `line`, calls `catch_one` for each of the 3 signals, and ignores `SIGPIPE`. Gives 0 if each call succeeds. |
| `os_signal_is_caught` | Gives the flag. |

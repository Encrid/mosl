# The tests of the os layer

This document gives the limits of some cases of `tests/shared/cases/os.toml`.

## The step of the wall clock

The case "a wait lasts its time while the wall clock steps back, simulated on Linux and macOS" checks a claim of `os_event_wait`.
That claim is in `docs/extern/os.h.md`, *Functions*. The case runs `tests/wait_test.c` with a step of 3600 seconds.

On Linux and macOS, the case simulates the step:

1. `tests/wait_test.c` defines its own `clock_gettime`. The layer is in the program, so each call of the layer gets this function.
2. For `CLOCK_REALTIME`, it gives the time of the C library minus the step. For each other clock, it gives the time of the C library.
3. The program makes 20 waits of 50 ms, and counts the waits that end too soon.

Thus a wait that reads the wall clock ends too soon, and the case fails.

On Windows, the layer reads no wall clock: `os_get_now` reads `QueryPerformanceCounter`, and `WaitForSingleObject` takes a time-out in milliseconds.
A test program cannot step the wall clock of Windows without the rights of an administrator.
Thus on Windows the program does not compile its `clock_gettime`, and the step has no effect. The case then claims only that the 20 waits last their time.

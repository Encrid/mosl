# The tests of the os layer

This document gives the limits of some cases of `tests/shared/cases/os.toml`.

## A mutex that waits

This section is about the row "os_mutex_lock waits while another thread holds the mutex" of the case `threads`.
It checks a claim of `os_mutex_lock` in `docs/extern/os.h.md`, *Functions*. The row runs `tests/threads_test.c` with `held`:

1. The main thread locks the mutex, and starts a second thread.
2. The second thread sets the event `trying`, then calls `os_mutex_lock`. With the mutex, it adds 1 to a count, unlocks, and sets the event `taken`.
3. The main thread waits for `trying`. Then it waits up to 100 ms for `taken`, and reads the count while it still holds the mutex.
4. A count of 0 gives `waited`. Then the main thread unlocks, joins the thread, and reads the count again: 1 gives `taken`.

The row proves the wait only when the second thread calls `os_mutex_lock` within the 100 ms after `trying`.
A thread that the system does not run in that time gives `waited` also with a lock that does not wait.
A lock that does not wait, with a thread that runs in time, gives `mutex: taken, then taken`, and the row fails.

## A wall clock behind the clock of the kernel

This section is about the case "a wall clock behind the clock of the kernel does not change a wait, simulated on Linux and macOS".
It checks a claim of `os_event_wait` in `docs/extern/os.h.md`, *Functions*. The case runs `tests/wait_test.c` with a wall clock that is 3600 seconds behind.

On Linux and macOS, the case simulates the difference:

1. `tests/wait_test.c` defines its own `clock_gettime`. The layer is in the program, so each call of the layer gets this function.
2. For `CLOCK_REALTIME`, it gives the time of the C library minus 3600 seconds. For each other clock, it gives the time of the C library.
3. The program makes 20 waits of 50 ms, and counts the waits that end too soon.

The kernel does not see this difference. Thus the case finds a layer that gives the kernel a deadline of the wall clock, for example with `pthread_cond_timedwait`.
Such a deadline is 3600 seconds in the past for the kernel. So the wait ends at once, as after a step forward of the wall clock, and the case fails.

A layer that only reads the wall clock sees no step, because the difference stays the same. The case does not find a step back either.
A step back makes such a wait last too long, and the case counts only the waits that end too soon.

On Windows, the layer reads no wall clock: `os_get_now` reads `QueryPerformanceCounter`, and `WaitForSingleObject` takes a time-out in milliseconds.
A test program cannot step the wall clock of Windows without the privilege `SE_SYSTEMTIME_NAME`. By default, only the groups Administrators and Local Service have it.
Thus on Windows the program does not compile its `clock_gettime`, and the difference has no effect. The case then claims only that the 20 waits last their time.

The sources of the wall clock of Windows:

- Microsoft, `SetSystemTime`: "The calling process must have the SE_SYSTEMTIME_NAME privilege." https://learn.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-setsystemtime
- Microsoft, Change the system time: "By default, members of the Administrators and Local Service groups have this right on workstations and servers." https://learn.microsoft.com/en-us/previous-versions/windows/it-pro/windows-10/security/threat-protection/security-policy-settings/change-the-system-time

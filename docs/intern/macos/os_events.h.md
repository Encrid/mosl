# src/macos/os_events.h

This header gives the event of a queue of macOS a name of its own, for `src/macos/process_exit.c`.
It includes `<sys/event.h>`.

## Types

`os_kevent_t` is `struct kevent`. The function `kevent` and the struct share one name.
`fail.py` renames the function with a macro after the last `#include` of a file, and that macro would rename the struct too.
The name of this header comes before that macro, so the file writes `os_kevent_t` and not `struct kevent`.
For the same reason, the file sets the fields of an event one at a time, and does not use the macro `EV_SET`.

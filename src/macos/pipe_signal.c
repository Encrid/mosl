/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <signal.h>

#include "../posix/os_types.h"

void os_take_pipe_signal(const sigset_t *pipe_only)
{
    int number = 0;

    if (os_is_pipe_waiting())
        sigwait(pipe_only, &number);
}

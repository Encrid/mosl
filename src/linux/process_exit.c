/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

#include "../posix/os_types.h"

int os_open_pipe(int ends[2])
{
    return (pipe2(ends, O_CLOEXEC));
}

void os_quiet_pipe([[maybe_unused]] int fd)
{
}

int os_watch_exit(os_process_t *p)
{
    p->exit = os_pidfd_open(p->pid);
    return (p->exit < 0 ? -1 : 0);
}

int os_wait_exit(os_process_t *p, uint64_t deadline)
{
    struct pollfd fd = {p->exit, POLLIN, 0};
    int ready = -1;

    do {
        ready = poll(&fd, 1, os_get_wait(deadline));
    } while (ready < 0 && errno == EINTR);
    if (ready < 0)
        return (-1);
    return (ready ? 0 : 1);
}

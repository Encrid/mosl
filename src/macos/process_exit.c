/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "../posix/os_types.h"
#include "os_events.h"

int os_open_pipe(int ends[2])
{
    if (pipe(ends))
        return (-1);
    fcntl(ends[0], F_SETFD, FD_CLOEXEC);
    fcntl(ends[1], F_SETFD, FD_CLOEXEC);
    return (0);
}

int os_watch_exit(os_process_t *p)
{
    os_kevent_t change = {};

    p->exit = kqueue();
    if (p->exit < 0)
        return (-1);
    change.ident = (uintptr_t)p->pid;
    change.filter = EVFILT_PROC;
    change.flags = EV_ADD;
    change.fflags = NOTE_EXIT;
    return (kevent(p->exit, &change, 1, nullptr, 0, nullptr) ? -1 : 0);
}

static int wait_once(os_process_t *p, uint64_t deadline)
{
    os_kevent_t event = {};
    int wait = os_get_wait(deadline);
    struct timespec limit = {wait / 1000, (long)(wait % 1000) * 1'000'000};

    return (kevent(p->exit, nullptr, 0, &event, 1,
        wait < 0 ? nullptr : &limit));
}

int os_wait_exit(os_process_t *p, uint64_t deadline)
{
    int got = -1;

    do {
        got = wait_once(p, deadline);
    } while (got < 0 && errno == EINTR);
    if (got < 0)
        return (-1);
    return (got ? 0 : 1);
}

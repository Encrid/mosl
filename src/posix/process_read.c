/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <errno.h>
#include <limits.h>
#include <poll.h>
#include <unistd.h>

#include "os_types.h"

constexpr uint64_t MILLI = 1'000'000;
constexpr uint64_t MOST = (uint64_t)INT_MAX * MILLI;

int os_get_wait(uint64_t deadline)
{
    uint64_t now = 0;

    if (deadline == OS_FOREVER)
        return (-1);
    now = os_get_now();
    if (now >= deadline)
        return (0);
    if (deadline - now >= MOST)
        return (INT_MAX);
    return ((int)((deadline - now + MILLI - 1) / MILLI));
}

static int take_bytes(os_process_t *p, int *fd, int stream,
    size_t *len)
{
    ssize_t got = read(*fd, p->bytes, sizeof(p->bytes));

    if (got < 0)
        return (-1);
    if (got > 0) {
        *len = (size_t)got;
        return (stream);
    }
    close(*fd);
    *fd = -1;
    return (1);
}

static int read_once(os_process_t *p, uint64_t deadline, size_t *len)
{
    struct pollfd fds[3] = {{p->wake[0], POLLIN, 0}, {p->output, POLLIN, 0},
        {p->errors, POLLIN, 0}};
    int *ends[3] = {nullptr, &p->output, &p->errors};
    int ready = 0;
    int first = 1 + (int)(p->turn++ % 2);

    if (p->output < 0 && p->errors < 0)
        return (0);
    ready = poll(fds, 3, os_get_wait(deadline));
    if (ready < 0)
        return (errno == EINTR ? 1 : -1);
    if (!ready)
        return (-2);
    if (fds[0].revents)
        return (0);
    if (!fds[first].revents)
        first = 3 - first;
    return (take_bytes(p, ends[first], first + 1, len));
}

int os_process_read(os_process_t *p, uint64_t nanos, const char **bytes,
    size_t *len)
{
    uint64_t deadline = nanos == OS_FOREVER ? OS_FOREVER : os_get_now() + nanos;
    int result = 1;

    while (result == 1)
        result = read_once(p, deadline, len);
    *bytes = p->bytes;
    return (result);
}

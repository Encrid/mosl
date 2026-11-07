/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <errno.h>
#include <signal.h>
#include <unistd.h>

#include "os_types.h"

static bool is_pipe_waiting(void)
{
    sigset_t waiting;

    sigemptyset(&waiting);
    sigpending(&waiting);
    return (sigismember(&waiting, SIGPIPE) == 1);
}

static ssize_t write_once(int fd, const char *bytes, size_t len)
{
    ssize_t wrote = write(fd, bytes, len);

    if (wrote < 0) {
        if (errno != EINTR)
            return (-1);
        return (0);
    }
    return (wrote);
}

static int write_each(int fd, const char *bytes, size_t len)
{
    ssize_t wrote = 0;

    while (len) {
        wrote = write_once(fd, bytes, len);
        if (wrote < 0)
            return (-1);
        bytes += wrote;
        len -= (size_t)wrote;
    }
    return (0);
}

int os_write_all(int fd, const void *bytes, size_t len)
{
    sigset_t pipe_only;
    sigset_t old;
    bool before = false;
    int number = 0;
    int wrote = 0;

    sigemptyset(&pipe_only);
    sigaddset(&pipe_only, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &pipe_only, &old);
    before = is_pipe_waiting();
    wrote = write_each(fd, bytes, len);
    if (wrote && errno == EPIPE && !before && is_pipe_waiting())
        sigwait(&pipe_only, &number);
    pthread_sigmask(SIG_SETMASK, &old, nullptr);
    return (wrote);
}

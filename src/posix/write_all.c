/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <unistd.h>

#include "os_types.h"

bool os_is_pipe_waiting(void)
{
    sigset_t waiting;

    sigemptyset(&waiting);
    sigpending(&waiting);
    return (sigismember(&waiting, SIGPIPE) == 1);
}

static int wait_room(int fd, int wake)
{
    struct pollfd fds[2] = {{fd, POLLOUT, 0}, {wake, POLLIN, 0}};
    int ready = poll(fds, 2, -1);

    if (ready < 0)
        return (errno == EINTR ? 0 : -1);
    return (fds[1].revents ? -1 : 0);
}

static ssize_t write_once(int fd, int wake, const char *bytes, size_t len)
{
    ssize_t wrote = write(fd, bytes, len);

    if (wrote >= 0)
        return (wrote);
    if (errno == EAGAIN)
        return (wait_room(fd, wake));
    return (errno == EINTR ? 0 : -1);
}

static int write_each(int fd, int wake, const char *bytes, size_t len)
{
    ssize_t wrote = 0;

    while (len) {
        wrote = write_once(fd, wake, bytes, len);
        if (wrote < 0)
            return (-1);
        bytes += wrote;
        len -= (size_t)wrote;
    }
    return (0);
}

int os_write_all(int fd, int wake, const void *bytes, size_t len)
{
    sigset_t pipe_only;
    sigset_t old;
    bool before = false;
    int wrote = 0;

    sigemptyset(&pipe_only);
    sigaddset(&pipe_only, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &pipe_only, &old);
    before = os_is_pipe_waiting();
    wrote = write_each(fd, wake, bytes, len);
    if (wrote && errno == EPIPE && !before)
        os_take_pipe_signal(&pipe_only);
    pthread_sigmask(SIG_SETMASK, &old, nullptr);
    return (wrote);
}

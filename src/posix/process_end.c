/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <sys/wait.h>
#include <unistd.h>

#include "os_types.h"

constexpr int NUMBERS[] = {0, SIGINT, SIGTERM, SIGHUP};

static int send_signal(os_process_t *p, int number)
{
    int sent = -1;

    pthread_mutex_lock(&p->lock);
    if (!p->taken)
        sent = kill(p->pid, number) ? -1 : 0;
    pthread_mutex_unlock(&p->lock);
    return (sent);
}

int os_process_signal(os_process_t *p, int which)
{
    if (!p->signals || p->hung)
        return (-1);
    p->hung = which == OS_HANG_UP;
    return (send_signal(p, NUMBERS[which]));
}

void os_process_kill(os_process_t *p)
{
    char wake = 1;
    [[maybe_unused]] ssize_t wrote = write(p->wake[1], &wake, 1);

    send_signal(p, SIGKILL);
}

int os_process_wait(os_process_t *p, uint64_t nanos, int64_t *status)
{
    uint64_t deadline = nanos == OS_FOREVER ? OS_FOREVER : os_get_now() + nanos;
    int ended = p->taken || p->ended ? 0 : os_wait_exit(p, deadline);
    siginfo_t info = {};

    if (ended)
        return (ended);
    p->ended = true;
    pthread_mutex_lock(&p->lock);
    if (!p->taken && !waitid(P_PID, (id_t)p->pid, &info, WEXITED)) {
        p->taken = true;
        p->status = info.si_status;
        if (info.si_code != CLD_EXITED)
            p->status = -p->status;
    }
    *status = p->status;
    ended = p->taken ? 0 : -1;
    pthread_mutex_unlock(&p->lock);
    return (ended);
}

void os_process_free(os_process_t *p)
{
    int64_t status = 0;
    [[maybe_unused]] int ended = 0;

    if (!p)
        return;
    if (!p->taken) {
        os_process_kill(p);
        ended = os_process_wait(p, OS_FOREVER, &status);
    }
    close(p->input);
    close(p->output);
    close(p->errors);
    pthread_mutex_destroy(&p->lock);
    os_drop_process(p);
}

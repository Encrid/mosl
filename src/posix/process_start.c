/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "os_types.h"

constexpr int CHILD_END[OS_PIPES] = {0, 1, 1, 0, 1};

static int open_pipes(os_process_t *p, const os_process_start_t *start,
    int pipes[][2])
{
    memset(p->wake, -1, sizeof(p->wake));
    p->exit = -1;
    for (int i = 0; i < OS_PIPES; i++) {
        if (i == OS_PIPE_ERRORS && !start->errors_apart)
            continue;
        if (os_open_pipe(pipes[i]))
            return (-1);
    }
    return (os_open_pipe(p->wake));
}

static void keep_ends(os_process_t *p, const os_process_start_t *start,
    int pipes[][2])
{
    p->input = pipes[OS_PIPE_INPUT][1];
    p->output = pipes[OS_PIPE_OUTPUT][0];
    p->errors = pipes[OS_PIPE_ERRORS][0];
    pipes[OS_PIPE_INPUT][1] = -1;
    pipes[OS_PIPE_OUTPUT][0] = -1;
    pipes[OS_PIPE_ERRORS][0] = -1;
    fcntl(p->input, F_SETFL, O_NONBLOCK);
    fcntl(p->output, F_SETFL, O_NONBLOCK);
    fcntl(p->errors, F_SETFL, O_NONBLOCK);
    fcntl(p->wake[1], F_SETFL, O_NONBLOCK);
    os_quiet_pipe(p->input);
    p->signals = start->signals;
    pthread_mutex_init(&p->lock, nullptr);
}

static int finish_start(os_process_t *p, const os_process_start_t *start,
    int pipes[][2])
{
    siginfo_t info = {};
    int code = 0;
    bool failed = false;

    for (int i = 0; i < OS_PIPES; i++) {
        close(pipes[i][CHILD_END[i]]);
        pipes[i][CHILD_END[i]] = -1;
    }
    if (os_watch_exit(p)) {
        kill(p->pid, SIGKILL);
        failed = true;
    }
    close(pipes[OS_PIPE_GATE][1]);
    pipes[OS_PIPE_GATE][1] = -1;
    failed |= read(pipes[OS_PIPE_REPORT][0], &code, sizeof(code)) > 0;
    if (failed)
        waitid(P_PID, (id_t)p->pid, &info, WEXITED);
    else
        keep_ends(p, start, pipes);
    return (failed ? -1 : 0);
}

static int start_child(os_process_t *p, const os_process_start_t *start,
    os_launch_t *launch)
{
    sigset_t all;
    sigset_t old;

    sigfillset(&all);
    pthread_sigmask(SIG_SETMASK, &all, &old);
    p->pid = fork();
    if (p->pid == 0)
        os_run_child(start, launch);
    pthread_sigmask(SIG_SETMASK, &old, nullptr);
    if (p->pid < 0)
        return (-1);
    return (finish_start(p, start, launch->pipes));
}

os_process_t *os_process_start(const os_process_start_t *start)
{
    os_process_t *p = calloc(1, sizeof(*p));
    os_launch_t launch = {};
    int started = -1;

    memset(launch.pipes, -1, sizeof(launch.pipes));
    launch.path = os_find_program(start->argv[0]);
    launch.envp = os_build_env(start->env, start->unset);
    if (p && launch.path && launch.envp && !open_pipes(p, start, launch.pipes))
        started = start_child(p, start, &launch);
    free(launch.path);
    free(launch.envp);
    os_close_pipes(launch.pipes, OS_PIPES);
    if (!started)
        return (p);
    if (p)
        os_drop_process(p);
    return (nullptr);
}

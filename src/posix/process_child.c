/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "os_types.h"

constexpr int RESET[] = {SIGINT, SIGTERM, SIGHUP, SIGPIPE};

static void set_signals(bool ignore)
{
    struct sigaction act = {};

    sigemptyset(&act.sa_mask);
    for (size_t i = 0; i < sizeof(RESET) / sizeof(RESET[0]); i++) {
        act.sa_handler = ignore && RESET[i] == SIGINT ? SIG_IGN : SIG_DFL;
        sigaction(RESET[i], &act, nullptr);
    }
}

[[noreturn]] static void fail_child(int report)
{
    int code = errno;
    [[maybe_unused]] ssize_t wrote = write(report, &code, sizeof(code));

    _exit(127);
}

static void put_end(int end, int stream)
{
    if (end == stream)
        fcntl(end, F_SETFD, 0);
    else
        dup2(end, stream);
}

void os_run_child(const os_process_start_t *start, const os_launch_t *launch)
{
    int output = launch->pipes[OS_PIPE_OUTPUT][1];
    int errors = launch->pipes[OS_PIPE_ERRORS][1];
    int report = launch->pipes[OS_PIPE_REPORT][1];
    [[maybe_unused]] ssize_t got = 0;
    char gate = 0;
    sigset_t none;

    set_signals(start->ignore_interrupt);
    close(launch->pipes[OS_PIPE_GATE][1]);
    got = read(launch->pipes[OS_PIPE_GATE][0], &gate, 1);
    put_end(launch->pipes[OS_PIPE_INPUT][0], STDIN_FILENO);
    put_end(output, STDOUT_FILENO);
    put_end(errors < 0 ? output : errors, STDERR_FILENO);
    if (start->folder && chdir(start->folder))
        fail_child(report);
    sigemptyset(&none);
    sigprocmask(SIG_SETMASK, &none, nullptr);
    execve(launch->path, (char *const *)start->argv, launch->envp);
    fail_child(report);
}

/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "os_types.h"

constexpr uint64_t MILLI = 1'000'000;
constexpr uint64_t MOST = (uint64_t)(INFINITE - 1) * MILLI;

DWORD os_get_wait(uint64_t deadline)
{
    uint64_t now = 0;

    if (deadline == OS_FOREVER)
        return (INFINITE);
    now = os_get_now();
    if (now >= deadline)
        return (0);
    if (deadline - now >= MOST)
        return (INFINITE - 1);
    return ((DWORD)((deadline - now + MILLI - 1) / MILLI));
}

void os_process_kill(os_process_t *p)
{
    SetEvent(p->stop);
    TerminateProcess(p->process, 1);
}

int os_process_wait(os_process_t *p, uint64_t nanos, int64_t *status)
{
    uint64_t deadline = nanos == OS_FOREVER ? OS_FOREVER : os_get_now() + nanos;
    DWORD waited = WaitForSingleObject(p->process, os_get_wait(deadline));
    DWORD code = 0;

    if (waited == WAIT_TIMEOUT)
        return (1);
    if (waited != WAIT_OBJECT_0 || !GetExitCodeProcess(p->process, &code))
        return (-1);
    *status = code;
    return (0);
}

void os_drop_process(os_process_t *p)
{
    HANDLE handles[] = {p->input, p->console_input, p->stop, p->writing,
        p->process, p->streams[0].read.hEvent, p->streams[1].read.hEvent};

    os_close_stream(&p->streams[0]);
    os_close_stream(&p->streams[1]);
    if (p->console)
        ClosePseudoConsole(p->console);
    for (size_t i = 0; i < sizeof(handles) / sizeof(handles[0]); i++) {
        if (handles[i])
            CloseHandle(handles[i]);
    }
    free(p);
}

void os_process_free(os_process_t *p)
{
    int64_t status = 0;
    [[maybe_unused]] int ended = 0;

    if (!p)
        return;
    if (WaitForSingleObject(p->process, 0) != WAIT_OBJECT_0) {
        os_process_kill(p);
        ended = os_process_wait(p, OS_FOREVER, &status);
    }
    os_drop_process(p);
}

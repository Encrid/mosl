/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "os_types.h"

constexpr DWORD_PTR HANDLES = PROC_THREAD_ATTRIBUTE_HANDLE_LIST;
constexpr DWORD_PTR CONSOLE = PROC_THREAD_ATTRIBUTE_PSEUDOCONSOLE;

static LPPROC_THREAD_ATTRIBUTE_LIST make_list(HANDLE *handles, DWORD count,
    HPCON console)
{
    DWORD attributes = console ? 2 : 1;
    SIZE_T size = 0;
    LPPROC_THREAD_ATTRIBUTE_LIST list = nullptr;

    InitializeProcThreadAttributeList(nullptr, attributes, 0, &size);
    list = malloc(size);
    if (!list || !InitializeProcThreadAttributeList(list, attributes, 0,
        &size)) {
        free(list);
        return (nullptr);
    }
    UpdateProcThreadAttribute(list, 0, HANDLES, handles,
        count * sizeof(HANDLE), nullptr, nullptr);
    if (console)
        UpdateProcThreadAttribute(list, 0, CONSOLE, console, sizeof(console),
            nullptr, nullptr);
    return (list);
}

static void free_list(LPPROC_THREAD_ATTRIBUTE_LIST list)
{
    if (!list)
        return;
    DeleteProcThreadAttributeList(list);
    free(list);
}

static int open_parts(os_process_t *p, const os_process_start_t *start,
    HANDLE child[3])
{
    HANDLE *events[] = {&p->stop, &p->writing, &p->streams[0].read.hEvent,
        &p->streams[1].read.hEvent};
    HANDLE *parents[] = {&p->input, &p->streams[0].pipe, &p->streams[1].pipe};
    HANDLE ends[2] = {};

    for (size_t i = 0; i < sizeof(events) / sizeof(events[0]); i++) {
        *events[i] = CreateEventW(nullptr, TRUE, FALSE, nullptr);
        if (!*events[i])
            return (-1);
    }
    for (int i = 0; i < 2 + start->errors_apart; i++) {
        if (os_open_pipe(p, i, i > 0, ends))
            return (-1);
        *parents[i] = ends[0];
        child[i] = ends[1];
    }
    p->signals = start->signals;
    if (start->signals && !start->ignore_interrupt)
        SetConsoleCtrlHandler(nullptr, FALSE);
    return (start->signals ? os_make_console(p) : 0);
}

static bool run_process(os_process_t *p, const os_process_start_t *start,
    os_launch_t *launch)
{
    HANDLE errors = launch->child[2] ? launch->child[2] : launch->child[1];
    STARTUPINFOEXW info = {.StartupInfo = {.cb = sizeof(info),
        .dwFlags = STARTF_USESTDHANDLES, .hStdInput = launch->child[0],
        .hStdOutput = launch->child[1], .hStdError = errors}};
    PROCESS_INFORMATION made = {};
    DWORD flags = EXTENDED_STARTUPINFO_PRESENT | CREATE_UNICODE_ENVIRONMENT
        | (start->ignore_interrupt ? CREATE_NEW_PROCESS_GROUP : 0);
    BOOL ran = FALSE;

    info.lpAttributeList = make_list(launch->child, 2 + start->errors_apart,
        p->console);
    if (info.lpAttributeList)
        ran = CreateProcessW(launch->program, launch->line, nullptr, nullptr,
            TRUE, flags, launch->block, launch->folder, &info.StartupInfo,
            &made);
    free_list(info.lpAttributeList);
    p->process = made.hProcess;
    if (ran)
        CloseHandle(made.hThread);
    return (ran);
}

os_process_t *os_process_start(const os_process_start_t *start)
{
    os_process_t *p = calloc(1, sizeof(*p));
    os_launch_t launch = {};
    bool started = false;

    if (p && os_prepare_launch(start, &launch)
        && !open_parts(p, start, launch.child))
        started = run_process(p, start, &launch);
    free(launch.program);
    free(launch.line);
    free(launch.block);
    free(launch.folder);
    for (int i = 0; i < 3; i++) {
        if (launch.child[i])
            CloseHandle(launch.child[i]);
    }
    if (started)
        return (p);
    if (p)
        os_drop_process(p);
    return (nullptr);
}

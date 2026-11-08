/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "os_types.h"

constexpr DWORD ROOM = 65536;
constexpr DWORD MOST_PART = 1u << 20;

static HANDLE open_server(const wchar_t *name, bool inbound)
{
    DWORD way = inbound ? PIPE_ACCESS_INBOUND : PIPE_ACCESS_OUTBOUND;
    HANDLE server = CreateNamedPipeW(name, way | FILE_FLAG_OVERLAPPED
        | FILE_FLAG_FIRST_PIPE_INSTANCE, PIPE_TYPE_BYTE | PIPE_WAIT
        | PIPE_REJECT_REMOTE_CLIENTS, 1, ROOM, ROOM, 0, nullptr);

    return (server == INVALID_HANDLE_VALUE ? nullptr : server);
}

int os_open_pipe(os_process_t *p, int index, bool inbound, HANDLE ends[2])
{
    SECURITY_ATTRIBUTES inherit = {sizeof(inherit), nullptr, TRUE};
    wchar_t name[96];

    swprintf(name, 96, L"\\\\.\\pipe\\mosl-%lu-%llx-%d",
        GetCurrentProcessId(), (unsigned long long)(uintptr_t)p, index);
    ends[0] = open_server(name, inbound);
    if (!ends[0])
        return (-1);
    ends[1] = CreateFileW(name, inbound ? GENERIC_WRITE : GENERIC_READ, 0,
        &inherit, OPEN_EXISTING, 0, nullptr);
    if (ends[1] != INVALID_HANDLE_VALUE)
        return (0);
    CloseHandle(ends[0]);
    ends[0] = nullptr;
    ends[1] = nullptr;
    return (-1);
}

static int write_part(os_process_t *p, const char *at, DWORD part,
    DWORD *wrote)
{
    OVERLAPPED over = {.hEvent = p->writing};
    HANDLE waits[2] = {p->writing, p->stop};

    if (!WriteFile(p->input, at, part, nullptr, &over)
        && GetLastError() != ERROR_IO_PENDING)
        return (-1);
    if (WaitForMultipleObjects(2, waits, FALSE, INFINITE) != WAIT_OBJECT_0)
        CancelIoEx(p->input, &over);
    return (GetOverlappedResult(p->input, &over, wrote, TRUE) ? 0 : -1);
}

int os_process_write(os_process_t *p, const void *bytes, size_t len)
{
    const char *at = bytes;
    DWORD wrote = 0;

    if (!p->input)
        return (-1);
    while (len) {
        if (write_part(p, at, len < MOST_PART ? (DWORD)len : MOST_PART,
            &wrote))
            return (-1);
        at += wrote;
        len -= wrote;
    }
    return (0);
}

void os_process_close(os_process_t *p, int stream)
{
    if (stream != OS_INPUT) {
        os_close_stream(&p->streams[stream - OS_OUTPUT]);
        return;
    }
    if (p->input)
        CloseHandle(p->input);
    p->input = nullptr;
}

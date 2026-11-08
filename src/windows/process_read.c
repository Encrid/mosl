/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "os_types.h"

static int start_read(os_stream_t *s)
{
    if (!s->pipe || s->pending)
        return (0);
    if (ReadFile(s->pipe, s->bytes, sizeof(s->bytes), nullptr, &s->read)
        || GetLastError() == ERROR_IO_PENDING) {
        s->pending = true;
        return (0);
    }
    if (GetLastError() != ERROR_BROKEN_PIPE)
        return (-1);
    os_close_stream(s);
    return (0);
}

static int take_bytes(os_process_t *p, int index, size_t *len)
{
    os_stream_t *s = &p->streams[index];
    DWORD got = 0;

    s->pending = false;
    if (!GetOverlappedResult(s->pipe, &s->read, &got, FALSE)) {
        if (GetLastError() != ERROR_BROKEN_PIPE)
            return (-1);
        os_close_stream(s);
        return (1);
    }
    *len = got;
    return (OS_OUTPUT + index);
}

static int queue_reads(os_process_t *p, HANDLE waits[3], int indexes[3])
{
    int first = (int)(p->turn++ % 2);
    int count = 1;
    int i = 0;

    for (int k = 0; k < 2; k++) {
        i = (first + k) % 2;
        if (start_read(&p->streams[i]))
            return (-1);
        if (p->streams[i].pending) {
            indexes[count] = i;
            waits[count++] = p->streams[i].read.hEvent;
        }
    }
    return (count);
}

static int read_once(os_process_t *p, uint64_t deadline, size_t *len)
{
    HANDLE waits[3] = {p->stop};
    int indexes[3] = {};
    int queued = queue_reads(p, waits, indexes);
    DWORD count = queued < 0 ? 0 : (DWORD)queued;
    DWORD got = 0;

    if (queued <= 1)
        return (queued < 0 ? -1 : 0);
    got = WaitForMultipleObjects(count, waits, FALSE, os_get_wait(deadline));
    if (got == WAIT_TIMEOUT)
        return (-2);
    if (got == WAIT_OBJECT_0 || got >= WAIT_OBJECT_0 + count)
        return (got == WAIT_OBJECT_0 ? 0 : -1);
    return (take_bytes(p, indexes[got - WAIT_OBJECT_0], len));
}

int os_process_read(os_process_t *p, uint64_t nanos, const char **bytes,
    size_t *len)
{
    uint64_t deadline = nanos == OS_FOREVER ? OS_FOREVER : os_get_now() + nanos;
    int result = 1;

    while (result == 1)
        result = read_once(p, deadline, len);
    *bytes = p->streams[result == OS_ERRORS].bytes;
    return (result);
}

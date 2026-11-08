/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "os_types.h"

void os_close_stream(os_stream_t *s)
{
    DWORD got = 0;

    if (s->pending) {
        CancelIoEx(s->pipe, &s->read);
        GetOverlappedResult(s->pipe, &s->read, &got, TRUE);
        s->pending = false;
    }
    if (s->pipe)
        CloseHandle(s->pipe);
    s->pipe = nullptr;
}

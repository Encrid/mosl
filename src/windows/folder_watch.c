/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <wchar.h>

#include "os_types.h"
#include "os_whole.h"

constexpr DWORD FLAGS = FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED;

static HANDLE open_listing(const wchar_t *path)
{
    return (CreateFileW(path, FILE_LIST_DIRECTORY, OS_SHARE_ALL, nullptr,
        OPEN_EXISTING, FLAGS, nullptr));
}

static bool keep_final(os_watch_t *w)
{
    DWORD size = GetFinalPathNameByHandleW(w->folder, nullptr, 0, 0);

    if (size)
        w->final = calloc(2 * (size_t)size, sizeof(*w->final));
    if (!w->final)
        return (false);
    w->again = w->final + size;
    return (GetFinalPathNameByHandleW(w->folder, w->final, size, 0)
        == size - 1);
}

static bool watch_above(os_watch_t *w)
{
    wchar_t *last = nullptr;
    wchar_t kept = L'\0';

    if (!keep_final(w))
        return (false);
    last = wcsrchr(w->final, L'\\');
    if (!last[1])
        return (true);
    kept = last[1];
    last[1] = L'\0';
    w->above = open_listing(w->final);
    last[1] = kept;
    return (w->above != INVALID_HANDLE_VALUE && os_read_above(w));
}

os_watch_t *os_watch_start(const char *folder, uint32_t depth)
{
    os_watch_t *w = depth ? calloc(1, sizeof(*w)) : nullptr;
    wchar_t *whole = nullptr;

    if (!w)
        return (nullptr);
    w->folder = INVALID_HANDLE_VALUE;
    w->above = INVALID_HANDLE_VALUE;
    w->depth = depth;
    whole = os_get_whole(folder);
    if (whole) {
        wcscat(whole, L"\\");
        w->folder = open_listing(whole);
    }
    free(whole);
    if (w->folder != INVALID_HANDLE_VALUE && watch_above(w)
        && os_read_changes(w))
        return (w);
    os_watch_stop(w);
    return (nullptr);
}

void os_watch_stop(os_watch_t *w)
{
    if (!w)
        return;
    if (w->reading)
        os_end_read(w->folder, &w->changes);
    if (w->looking)
        os_end_read(w->above, &w->above_changes);
    if (w->folder != INVALID_HANDLE_VALUE)
        CloseHandle(w->folder);
    if (w->above != INVALID_HANDLE_VALUE)
        CloseHandle(w->above);
    free(w->final);
    free(w);
}

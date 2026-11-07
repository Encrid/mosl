/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <wchar.h>

#include "os_types.h"
#include "os_whole.h"

wchar_t *os_get_whole(const char *path)
{
    wchar_t *wide = os_widen(path);
    wchar_t *whole = nullptr;
    DWORD size = 0;

    if (wide)
        size = GetFullPathNameW(wide, 0, nullptr, nullptr);
    if (size)
        whole = malloc((OS_PLACE + size + 1 + OS_NAME_ROOM) * sizeof(*whole));
    if (whole) {
        GetFullPathNameW(wide, size, whole + OS_PLACE, nullptr);
        os_mark_whole(whole, size);
    }
    free(wide);
    return (whole);
}

os_folder_t *os_folder_open(const char *path)
{
    os_folder_t *f = calloc(1, sizeof(*f));
    size_t len = 0;

    if (!f)
        return (nullptr);
    f->find = INVALID_HANDLE_VALUE;
    f->whole = os_get_whole(path);
    if (f->whole) {
        len = wcslen(f->whole);
        wmemcpy(f->whole + len, L"\\*", 3);
        f->find = FindFirstFileExW(f->whole, FindExInfoBasic, &f->data,
            FindExSearchNameMatch, nullptr, 0);
        f->whole[len] = L'\0';
    }
    f->pending = f->find != INVALID_HANDLE_VALUE;
    if (f->pending)
        return (f);
    os_folder_close(f);
    return (nullptr);
}

void os_folder_close(os_folder_t *f)
{
    if (!f)
        return;
    if (f->find != INVALID_HANDLE_VALUE)
        FindClose(f->find);
    free(f->whole);
    free(f->bytes);
    free(f);
}

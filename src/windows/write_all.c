/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "os_types.h"

constexpr DWORD MOST = 1u << 20;

int os_write_all(HANDLE handle, const void *bytes, size_t len)
{
    const char *at = bytes;
    DWORD part = 0;
    DWORD wrote = 0;

    while (len) {
        part = len < MOST ? (DWORD)len : MOST;
        if (!WriteFile(handle, at, part, &wrote, nullptr))
            return (-1);
        at += wrote;
        len -= wrote;
    }
    return (0);
}

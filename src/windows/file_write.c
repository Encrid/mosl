/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <wchar.h>

#include "os_types.h"
#include "os_whole.h"

int os_file_write(const char *path, const void *bytes, size_t len,
    [[maybe_unused]] bool executable)
{
    wchar_t *whole = os_get_whole(path);
    HANDLE file = INVALID_HANDLE_VALUE;
    int wrote = -1;

    if (whole)
        file = CreateFileW(whole, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file != INVALID_HANDLE_VALUE) {
        wrote = os_write_all(file, bytes, len);
        if (!CloseHandle(file))
            wrote = -1;
    }
    free(whole);
    return (wrote);
}

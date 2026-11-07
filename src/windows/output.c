/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "os_types.h"

static int write_console(HANDLE console, const char *bytes, size_t len)
{
    int count = MultiByteToWideChar(CP_UTF8, 0, bytes, (int)len, nullptr, 0);
    wchar_t *wide = malloc(((size_t)count + 1) * sizeof(*wide));
    DWORD wrote = 0;
    int at = 0;

    if (!wide)
        return (-1);
    MultiByteToWideChar(CP_UTF8, 0, bytes, (int)len, wide, count);
    while (at < count && WriteConsoleW(console, wide + at,
        (DWORD)(count - at), &wrote, nullptr))
        at += (int)wrote;
    free(wide);
    return (at == count ? 0 : -1);
}

int os_output_write(const char *bytes, size_t len)
{
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;

    if (output == INVALID_HANDLE_VALUE || !output)
        return (-1);
    if (GetConsoleMode(output, &mode))
        return (write_console(output, bytes, len));
    return (os_write_all(output, bytes, len));
}

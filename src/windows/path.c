/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "os_types.h"

char *os_whole_text(const char *path)
{
    wchar_t *wide = os_widen(path);
    DWORD size = wide ? GetFullPathNameW(wide, 0, nullptr, nullptr) : 0;
    wchar_t *whole = size ? malloc(size * sizeof(*whole)) : nullptr;
    char *text = nullptr;

    if (whole && GetFullPathNameW(wide, size, whole, nullptr))
        text = os_narrow(whole);
    free(wide);
    free(whole);
    return (text);
}

char *os_path_whole(const char *path)
{
    return (os_whole_text(path));
}

/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <wchar.h>

#include "os_whole.h"

void os_mark_whole(wchar_t *whole, size_t size)
{
    size_t end = 0;

    if (whole[OS_PLACE] != L'\\') {
        wmemmove(whole + 4, whole + OS_PLACE, size);
        wmemcpy(whole, L"\\\\?\\", 4);
    } else if (!wcsncmp(whole + OS_PLACE, L"\\\\?\\", 4)
        || !wcsncmp(whole + OS_PLACE, L"\\\\.\\", 4)) {
        wmemmove(whole, whole + OS_PLACE, size);
    } else {
        wmemmove(whole + OS_PLACE, whole + OS_PLACE + 1, size - 1);
        wmemcpy(whole, L"\\\\?\\UNC", OS_PLACE);
    }
    end = wcslen(whole);
    if (whole[end - 1] == L'\\')
        whole[end - 1] = L'\0';
}

size_t os_get_root_end(const wchar_t *whole)
{
    int left = wcsncmp(whole, L"\\\\?\\UNC\\", 8) ? 4 : 6;
    size_t at = 0;

    for (at = 0; whole[at]; at++) {
        if (whole[at] == L'\\')
            left--;
        if (!left)
            break;
    }
    return (at);
}

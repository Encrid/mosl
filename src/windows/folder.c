/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <wchar.h>

#include "os_types.h"
#include "os_whole.h"

int os_folder_set(const char *folder)
{
    wchar_t *wide = os_widen(folder);
    int set = wide && SetCurrentDirectoryW(wide) ? 0 : -1;

    free(wide);
    return (set);
}

static int make_one(const wchar_t *whole)
{
    DWORD attributes = 0;

    if (CreateDirectoryW(whole, nullptr))
        return (0);
    attributes = GetFileAttributesW(whole);
    if (attributes == INVALID_FILE_ATTRIBUTES)
        return (-1);
    return (attributes & FILE_ATTRIBUTE_DIRECTORY ? 0 : -1);
}

int os_folder_make(const char *folder)
{
    wchar_t *whole = os_get_whole(folder);
    size_t at = whole ? os_get_root_end(whole) : 0;
    int made = whole ? 0 : -1;
    size_t end = 0;
    wchar_t kept = L'\0';

    while (!made && whole[at]) {
        end = at + 1 + wcscspn(whole + at + 1, L"\\");
        kept = whole[end];
        whole[end] = L'\0';
        made = make_one(whole);
        whole[end] = kept;
        at = end;
    }
    free(whole);
    return (made);
}

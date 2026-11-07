/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <wchar.h>

#include "os_types.h"
#include "os_whole.h"

constexpr DWORD SHARE_ALL = FILE_SHARE_READ | FILE_SHARE_WRITE
    | FILE_SHARE_DELETE;

static int get_whole_entry(const wchar_t *whole)
{
    HANDLE file = CreateFileW(whole, 0, SHARE_ALL, nullptr, OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS, nullptr);
    DWORD error = GetLastError();
    BY_HANDLE_FILE_INFORMATION info = {};
    int kind = -1;

    if (file == INVALID_HANDLE_VALUE)
        return (error == ERROR_FILE_NOT_FOUND
            || error == ERROR_PATH_NOT_FOUND ? 0 : -1);
    if (GetFileInformationByHandle(file, &info))
        kind = info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? 3 : 1;
    CloseHandle(file);
    return (kind);
}

int os_path_entry(const char *path)
{
    wchar_t *whole = os_get_whole(path);
    int kind = whole ? get_whole_entry(whole) : -1;

    free(whole);
    return (kind);
}

int os_path_move(const char *from, const char *to)
{
    wchar_t *from_whole = os_get_whole(from);
    wchar_t *to_whole = os_get_whole(to);
    int moved = -1;

    if (from_whole && to_whole
        && MoveFileExW(from_whole, to_whole, MOVEFILE_REPLACE_EXISTING))
        moved = 0;
    free(from_whole);
    free(to_whole);
    return (moved);
}

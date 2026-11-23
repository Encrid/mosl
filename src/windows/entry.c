/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "os_types.h"
#include "os_whole.h"
#include "../common/os_common.h"

constexpr DWORD OPEN_AS_ENTRY = FILE_FLAG_BACKUP_SEMANTICS
    | FILE_FLAG_OPEN_REPARSE_POINT;

int os_get_whole_info(const wchar_t *whole, BY_HANDLE_FILE_INFORMATION *info)
{
    HANDLE file = CreateFileW(whole, 0, OS_SHARE_ALL, nullptr, OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS, nullptr);
    DWORD error = GetLastError();
    int found = -1;

    if (file == INVALID_HANDLE_VALUE)
        return (error == ERROR_FILE_NOT_FOUND
            || error == ERROR_PATH_NOT_FOUND ? 0 : -1);
    if (GetFileInformationByHandle(file, info))
        found = 1;
    CloseHandle(file);
    return (found);
}

int os_path_entry(const char *path)
{
    wchar_t *whole = os_get_whole(path);
    BY_HANDLE_FILE_INFORMATION info = {};
    int found = whole ? os_get_whole_info(whole, &info) : -1;

    free(whole);
    if (found != 1)
        return (found);
    return (info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? 3 : 1);
}

static bool rename_posix(const wchar_t *from, const wchar_t *to)
{
    DWORD bytes = (DWORD)(wcslen(to) * sizeof(wchar_t));
    FILE_RENAME_INFO *info = calloc(1, sizeof(*info) + bytes);
    HANDLE handle = INVALID_HANDLE_VALUE;
    bool renamed = false;

    if (info)
        handle = CreateFileW(from, DELETE, OS_SHARE_ALL, nullptr,
            OPEN_EXISTING, OPEN_AS_ENTRY, nullptr);
    if (handle != INVALID_HANDLE_VALUE) {
        info->Flags = FILE_RENAME_FLAG_REPLACE_IF_EXISTS
            | FILE_RENAME_FLAG_POSIX_SEMANTICS;
        info->FileNameLength = bytes;
        memcpy(info->FileName, to, bytes);
        renamed = SetFileInformationByHandle(handle, FileRenameInfoEx, info,
            (DWORD)sizeof(*info) + bytes);
        CloseHandle(handle);
    }
    free(info);
    return (renamed);
}

int os_path_move(const char *from, const char *to, uint32_t flags)
{
    bool known = !(flags & ~OS_MOVE_FLAGS);
    wchar_t *from_whole = known ? os_get_whole(from) : nullptr;
    wchar_t *to_whole = known ? os_get_whole(to) : nullptr;
    DWORD how = MOVEFILE_REPLACE_EXISTING;
    int moved = -1;

    if (flags & OS_MOVE_DURABLE)
        how |= MOVEFILE_WRITE_THROUGH;
    if (from_whole && to_whole && (rename_posix(from_whole, to_whole)
        || MoveFileExW(from_whole, to_whole, how)))
        moved = 0;
    free(from_whole);
    free(to_whole);
    return (moved);
}

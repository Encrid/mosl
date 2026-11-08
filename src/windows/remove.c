/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <wchar.h>

#include "os_types.h"
#include "os_whole.h"

static wchar_t *join_path(const wchar_t *folder, const wchar_t *name)
{
    size_t at = wcslen(folder);
    size_t len = wcslen(name);
    wchar_t *path = malloc((at + len + 2) * sizeof(*path));

    if (!path)
        return (nullptr);
    wmemcpy(path, folder, at);
    path[at] = L'\\';
    wmemcpy(path + at + 1, name, len + 1);
    return (path);
}

static int remove_child(const wchar_t *folder, const wchar_t *name)
{
    wchar_t *child = join_path(folder, name);
    int removed = child ? os_remove_entry(child) : -1;

    free(child);
    return (removed);
}

static int remove_entries(const wchar_t *whole)
{
    wchar_t *pattern = join_path(whole, L"*");
    WIN32_FIND_DATAW data = {};
    HANDLE find = INVALID_HANDLE_VALUE;
    int removed = 0;
    BOOL more = false;

    if (pattern)
        find = FindFirstFileExW(pattern, FindExInfoBasic, &data,
            FindExSearchNameMatch, nullptr, 0);
    more = find != INVALID_HANDLE_VALUE;
    removed = more ? 0 : -1;
    while (!removed && more) {
        if (wcscmp(data.cFileName, L".") && wcscmp(data.cFileName, L".."))
            removed = remove_child(whole, data.cFileName);
        more = FindNextFileW(find, &data);
    }
    if (find != INVALID_HANDLE_VALUE)
        FindClose(find);
    free(pattern);
    return (removed);
}

int os_remove_entry(const wchar_t *whole)
{
    DWORD attributes = GetFileAttributesW(whole);

    if (attributes == INVALID_FILE_ATTRIBUTES)
        return (-1);
    if ((attributes & FILE_ATTRIBUTE_READONLY) && !SetFileAttributesW(whole,
        attributes & ~(DWORD)FILE_ATTRIBUTE_READONLY))
        return (-1);
    if (!(attributes & FILE_ATTRIBUTE_DIRECTORY))
        return (DeleteFileW(whole) ? 0 : -1);
    if (!(attributes & FILE_ATTRIBUTE_REPARSE_POINT) && remove_entries(whole))
        return (-1);
    return (RemoveDirectoryW(whole) ? 0 : -1);
}

int os_path_remove(const char *path)
{
    wchar_t *whole = os_get_whole(path);
    DWORD error = ERROR_SUCCESS;
    int removed = -1;

    if (whole && GetFileAttributesW(whole) == INVALID_FILE_ATTRIBUTES) {
        error = GetLastError();
        if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND)
            removed = 1;
    } else if (whole)
        removed = os_remove_entry(whole);
    free(whole);
    return (removed);
}

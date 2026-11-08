/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "os_types.h"
#include "os_whole.h"

static bool is_absolute(const char *target)
{
    if (target[0] == '/' || target[0] == '\\')
        return (true);
    return (target[0] && target[1] == ':');
}

static int get_target_kind(const char *path, const char *target)
{
    size_t folder = is_absolute(target) ? 0 : strlen(path);
    char *joined = malloc(folder + strlen(target) + 2);
    wchar_t *whole = nullptr;
    DWORD attributes = INVALID_FILE_ATTRIBUTES;
    int kind = -1;

    while (folder > 0 && !strchr("/\\", path[folder - 1]))
        folder--;
    if (joined)
        sprintf(joined, "%.*s%s", (int)folder, path, target);
    whole = joined ? os_get_whole(joined) : nullptr;
    if (whole) {
        attributes = GetFileAttributesW(whole);
        kind = attributes != INVALID_FILE_ATTRIBUTES
            && (attributes & FILE_ATTRIBUTE_DIRECTORY);
    }
    free(joined);
    free(whole);
    return (kind);
}

int os_link_make(const char *path, const char *target)
{
    wchar_t *whole = os_get_whole(path);
    wchar_t *aim = os_widen(target);
    DWORD flags = SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE;
    int kind = get_target_kind(path, target);
    int linked = -1;

    if (kind == 1)
        flags |= SYMBOLIC_LINK_FLAG_DIRECTORY;
    for (size_t i = 0; aim && aim[i]; i++)
        aim[i] = aim[i] == L'/' ? L'\\' : aim[i];
    if (whole && aim && *target && kind >= 0
        && CreateSymbolicLinkW(whole, aim, flags))
        linked = 0;
    free(whole);
    free(aim);
    return (linked);
}

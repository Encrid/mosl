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

static bool is_program(const char *path)
{
    wchar_t *whole = os_get_whole(path);
    DWORD attributes = INVALID_FILE_ATTRIBUTES;

    if (whole)
        attributes = GetFileAttributesW(whole);
    free(whole);
    if (attributes == INVALID_FILE_ATTRIBUTES)
        return (false);
    return (!(attributes & FILE_ATTRIBUTE_DIRECTORY));
}

static char *find_with(char *path, size_t at, const char *exts)
{
    const char *ext = exts;
    size_t len = 0;

    while (ext) {
        len = strcspn(ext, ";");
        memcpy(path + at, ext, len);
        path[at + len] = '\0';
        if (len && is_program(path))
            return (os_whole_text(path));
        ext = ext[len] ? ext + len + 1 : nullptr;
    }
    path[at] = '\0';
    return (is_program(path) ? os_whole_text(path) : nullptr);
}

static char *find_in(const char *folders, const char *name, const char *exts)
{
    size_t size = strlen(folders) + strlen(name) + strlen(exts) + 3;
    char *path = malloc(size);
    const char *item = folders;
    char *found = nullptr;
    size_t len = 0;

    while (path && !found && item) {
        len = strcspn(item, ";");
        if (len) {
            snprintf(path, size, "%.*s\\%s", (int)len, item, name);
            found = find_with(path, strlen(path), exts);
        }
        item = item[len] ? item + len + 1 : nullptr;
    }
    free(path);
    return (found);
}

char *os_find_text(const char *name)
{
    char *folders = os_get_variable("PATH");
    char *exts = strchr(name, '.') ? nullptr : os_get_variable("PATHEXT");
    char *found = nullptr;

    if (strpbrk(name, "\\/"))
        found = is_program(name) ? os_whole_text(name) : nullptr;
    else if (folders)
        found = find_in(folders, name, exts ? exts : "");
    free(folders);
    free(exts);
    return (found);
}

char *os_path_find(const char *name)
{
    return (os_find_text(name));
}

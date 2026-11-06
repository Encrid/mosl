/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "os.h"

static bool is_program(const char *path)
{
    struct stat st = {};

    if (stat(path, &st) || !S_ISREG(st.st_mode))
        return (false);
    return ((st.st_mode & 0111) != 0);
}

char *os_env_get(const char *name)
{
    const char *value = getenv(name);

    return (value ? strdup(value) : nullptr);
}

static char *find_in(const char *folders, const char *name)
{
    size_t size = strlen(folders) + strlen(name) + 3;
    char *path = malloc(size);
    const char *item = folders;
    bool matched = false;
    char *found = nullptr;
    size_t len = 0;

    while (path && !matched && item) {
        len = strcspn(item, ":");
        if (len)
            snprintf(path, size, "%.*s/%s", (int)len, item, name);
        else
            snprintf(path, size, "./%s", name);
        matched = is_program(path);
        item = item[len] ? item + len + 1 : nullptr;
    }
    found = matched ? realpath(path, nullptr) : nullptr;
    free(path);
    return (found);
}

char *os_path_find(const char *name)
{
    const char *folders = getenv("PATH");

    if (strchr(name, '/'))
        return (is_program(name) ? realpath(name, nullptr) : nullptr);
    return (folders ? find_in(folders, name) : nullptr);
}

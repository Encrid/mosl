/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "os_types.h"

constexpr int READABLE = O_RDONLY | O_DIRECTORY | O_CLOEXEC;

static int step(int at, const char *name, bool make)
{
    int next = -1;

    if (make)
        mkdirat(at, name, 0777);
    next = os_open_search(at, name);
    close(at);
    return (next);
}

static size_t get_last_start(const char *path)
{
    size_t end = strlen(path);

    while (end > 1 && path[end - 1] == '/')
        end--;
    while (end > 0 && path[end - 1] != '/')
        end--;
    return (end);
}

static char *start_copy(const char *path, size_t last, int *at)
{
    char *names = malloc(last + 1);

    if (!names)
        return (nullptr);
    memcpy(names, path, last);
    names[last] = '\0';
    if (last <= 1)
        *at = open("/", READABLE);
    else
        *at = os_open_search(AT_FDCWD, *path == '/' ? "/" : ".");
    return (names);
}

int os_open_parent(const char *path, const char **name, bool make)
{
    size_t last = get_last_start(path);
    char *names = nullptr;
    char *place = nullptr;
    char *item = nullptr;
    int at = -1;

    *name = path;
    if (!make && strlen(path) < PATH_MAX)
        return (AT_FDCWD);
    *name = path[last] > '/' ? path + last : ".";
    names = start_copy(path, last, &at);
    item = names ? strtok_r(names, "/", &place) : nullptr;
    while (item && at >= 0) {
        at = step(at, item, make);
        item = strtok_r(nullptr, "/", &place);
    }
    free(names);
    return (at);
}

void os_close_parent(int at)
{
    if (at >= 0)
        close(at);
}

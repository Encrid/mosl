/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <sched.h>
#include <stdlib.h>
#include <unistd.h>

#include "../posix/os_types.h"

constexpr size_t FIRST_SIZE = 16;

char *os_get_program_path(void)
{
    size_t size = FIRST_SIZE;
    char *path = malloc(size);
    char *grown = nullptr;
    ssize_t len = path ? readlink("/proc/self/exe", path, size) : -1;

    while (path && len >= 0 && (size_t)len == size) {
        size *= 2;
        grown = realloc(path, size);
        if (!grown)
            free(path);
        path = grown;
        len = path ? readlink("/proc/self/exe", path, size) : -1;
    }
    if (path && len >= 0) {
        path[len] = '\0';
        return (path);
    }
    free(path);
    return (nullptr);
}

uint32_t os_get_processor_count(void)
{
    cpu_set_t set = {};

    if (!sched_getaffinity(0, sizeof(set), &set))
        return ((uint32_t)__builtin_popcountg(&set, sizeof(set)));
    return (os_get_online_count());
}

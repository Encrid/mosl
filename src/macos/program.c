/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <limits.h>
#include <mach-o/dyld.h>
#include <stdlib.h>

#include "../posix/os_types.h"

char *os_get_program_path(void)
{
    uint32_t size = 0;
    char *path = nullptr;
    char *whole = nullptr;

    _NSGetExecutablePath(nullptr, &size);
    path = malloc(size);
    whole = malloc(PATH_MAX);
    if (path && whole && !_NSGetExecutablePath(path, &size)
        && realpath(path, whole)) {
        free(path);
        return (whole);
    }
    free(path);
    free(whole);
    return (nullptr);
}

uint32_t os_get_processor_count(void)
{
    return (os_get_online_count());
}

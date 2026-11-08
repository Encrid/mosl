/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "os_types.h"

constexpr DWORD FIRST_SIZE = 16;

uint32_t os_get_processor_count(void)
{
    DWORD_PTR process = 0;
    DWORD_PTR system = 0;

    if (GetProcessAffinityMask(GetCurrentProcess(), &process, &system)
        && process != system)
        return ((uint32_t)__builtin_popcountg(process));
    return ((uint32_t)GetActiveProcessorCount(ALL_PROCESSOR_GROUPS));
}

char *os_get_program_path(void)
{
    DWORD size = FIRST_SIZE;
    wchar_t *path = malloc(size * sizeof(*path));
    wchar_t *grown = nullptr;
    DWORD len = path ? GetModuleFileNameW(nullptr, path, size) : 0;
    char *text = nullptr;

    while (path && len == size) {
        size *= 2;
        grown = realloc(path, size * sizeof(*path));
        if (!grown)
            free(path);
        path = grown;
        len = path ? GetModuleFileNameW(nullptr, path, size) : 0;
    }
    if (path && len)
        text = os_narrow(path);
    free(path);
    return (text);
}

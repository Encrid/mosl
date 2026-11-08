/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "os_types.h"

char *os_get_variable(const char *name)
{
    wchar_t *wide = os_widen(name);
    DWORD size = wide ? GetEnvironmentVariableW(wide, nullptr, 0) : 0;
    wchar_t *value = size ? malloc(size * sizeof(*value)) : nullptr;
    char *text = nullptr;

    if (value && GetEnvironmentVariableW(wide, value, size) == size - 1)
        text = os_narrow(value);
    free(wide);
    free(value);
    return (text);
}

char *os_env_get(const char *name)
{
    return (os_get_variable(name));
}

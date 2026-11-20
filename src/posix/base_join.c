/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "os_types.h"

char *os_join_base(const char *base, const char *rest)
{
    bool absolute = base && base[0] == '/';
    size_t size = absolute ? strlen(base) + strlen(rest) + 1 : 0;
    char *path = absolute ? malloc(size) : nullptr;

    if (path)
        snprintf(path, size, "%s%s", base, rest);
    return (path);
}

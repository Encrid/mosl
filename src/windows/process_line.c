/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "os_types.h"

static bool needs_quotes(const char *arg)
{
    return (!*arg || strpbrk(arg, " \t\n\v\"") != nullptr);
}

static size_t add_slashes(char *out, size_t count)
{
    memset(out, '\\', count);
    return (count);
}

static size_t add_arg(char *out, const char *arg)
{
    size_t at = 0;
    size_t slashes = 0;

    if (!needs_quotes(arg)) {
        memcpy(out, arg, strlen(arg));
        return (strlen(arg));
    }
    out[at++] = '"';
    for (const char *c = arg; *c; c++) {
        if (*c == '"')
            at += add_slashes(out + at, slashes + 1);
        slashes = *c == '\\' ? slashes + 1 : 0;
        out[at++] = *c;
    }
    at += add_slashes(out + at, slashes);
    out[at++] = '"';
    return (at);
}

wchar_t *os_join_line(const char *const *argv)
{
    size_t size = 1;
    size_t at = 0;
    char *line = nullptr;
    wchar_t *wide = nullptr;

    for (size_t i = 0; argv[i]; i++)
        size += 2 * strlen(argv[i]) + 3;
    line = malloc(size);
    if (!line)
        return (nullptr);
    for (size_t i = 0; argv[i]; i++) {
        line[at] = ' ';
        at += i > 0;
        at += add_arg(line + at, argv[i]);
    }
    line[at] = '\0';
    wide = os_widen(line);
    free(line);
    return (wide);
}

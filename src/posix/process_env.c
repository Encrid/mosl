/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "os_types.h"

static bool has_name(const char *entry, const char *const *names)
{
    size_t len = strcspn(entry, "=");

    for (size_t i = 0; names && names[i]; i++) {
        if (!strncmp(entry, names[i], len)
            && (names[i][len] == '=' || names[i][len] == '\0'))
            return (true);
    }
    return (false);
}

static size_t count_texts(const char *const *texts)
{
    size_t count = 0;

    while (texts && texts[count])
        count++;
    return (count);
}

char **os_build_env(const char *const *env, const char *const *unset)
{
    extern char **environ;
    size_t size = count_texts((const char *const *)environ) + count_texts(env);
    char **out = malloc((size + 1) * sizeof(*out));
    size_t at = 0;

    if (!out)
        return (nullptr);
    for (size_t i = 0; environ[i]; i++) {
        if (!has_name(environ[i], env) && !has_name(environ[i], unset))
            out[at++] = environ[i];
    }
    for (size_t i = 0; env && env[i]; i++)
        out[at++] = (char *)env[i];
    out[at] = nullptr;
    return (out);
}

char *os_find_program(const char *name)
{
    size_t size = strlen(name) + 1;
    char *copy = nullptr;

    if (!strchr(name, '/'))
        return (os_path_find(name));
    copy = malloc(size);
    if (copy)
        memcpy(copy, name, size);
    return (copy);
}

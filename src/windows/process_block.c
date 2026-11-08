/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <wchar.h>

#include "os_types.h"

static size_t get_name_len(const wchar_t *entry)
{
    const wchar_t *end = wcschr(entry + 1, L'=');

    return (end ? (size_t)(end - entry) : wcslen(entry));
}

static bool has_name(const wchar_t *entry, wchar_t *const *names)
{
    size_t len = get_name_len(entry);

    for (size_t i = 0; names[i]; i++) {
        if (get_name_len(names[i]) == len && CompareStringOrdinal(entry,
            (int)len, names[i], (int)len, TRUE) == CSTR_EQUAL)
            return (true);
    }
    return (false);
}

static size_t copy_block(wchar_t *out, const wchar_t *system,
    wchar_t *const *env, wchar_t *const *unset)
{
    size_t at = 0;

    for (const wchar_t *e = system; *e; e += wcslen(e) + 1) {
        if (has_name(e, env) || has_name(e, unset))
            continue;
        if (out)
            wmemcpy(out + at, e, wcslen(e) + 1);
        at += wcslen(e) + 1;
    }
    for (size_t i = 0; env[i]; i++) {
        if (out)
            wmemcpy(out + at, env[i], wcslen(env[i]) + 1);
        at += wcslen(env[i]) + 1;
    }
    return (at);
}

wchar_t *os_build_block(const char *const *env, const char *const *unset)
{
    wchar_t **wide_env = os_widen_texts(env);
    wchar_t **wide_unset = os_widen_texts(unset);
    wchar_t *system = GetEnvironmentStringsW();
    wchar_t *block = nullptr;
    size_t size = 0;

    if (wide_env && wide_unset && system) {
        size = copy_block(nullptr, system, wide_env, wide_unset);
        block = malloc((size + 1) * sizeof(*block));
    }
    if (block) {
        copy_block(block, system, wide_env, wide_unset);
        block[size] = L'\0';
    }
    FreeEnvironmentStringsW(system);
    os_free_texts(wide_env);
    os_free_texts(wide_unset);
    return (block);
}

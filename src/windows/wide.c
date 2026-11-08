/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "os_types.h"

wchar_t *os_widen(const char *text)
{
    int count = MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0);
    wchar_t *wide = malloc((size_t)count * sizeof(*wide));

    if (wide)
        MultiByteToWideChar(CP_UTF8, 0, text, -1, wide, count);
    return (wide);
}

char *os_narrow(const wchar_t *wide)
{
    int size = WideCharToMultiByte(CP_UTF8, 0, wide, -1, nullptr, 0, nullptr,
        nullptr);
    char *text = malloc((size_t)size);

    if (text)
        WideCharToMultiByte(CP_UTF8, 0, wide, -1, text, size, nullptr,
            nullptr);
    return (text);
}

void os_free_texts(wchar_t **texts)
{
    for (size_t i = 0; texts && texts[i]; i++)
        free(texts[i]);
    free(texts);
}

wchar_t **os_widen_texts(const char *const *texts)
{
    size_t count = 0;
    wchar_t **wide = nullptr;
    bool failed = false;

    while (texts && texts[count])
        count++;
    wide = calloc(count + 1, sizeof(*wide));
    for (size_t i = 0; wide && i < count; i++) {
        wide[i] = os_widen(texts[i]);
        failed |= !wide[i];
    }
    if (!failed)
        return (wide);
    for (size_t i = 0; i < count; i++)
        free(wide[i]);
    free(wide);
    return (nullptr);
}

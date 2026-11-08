/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef MOSL_OS_WHOLE_H
    #define MOSL_OS_WHOLE_H

    #include <stddef.h>

constexpr size_t OS_PLACE = 7;

void os_mark_whole(wchar_t *whole, size_t size);

size_t os_get_root_end(const wchar_t *whole);

wchar_t *os_get_whole(const char *path);

#endif

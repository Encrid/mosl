/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef MOSL_OS_COMMON_H
    #define MOSL_OS_COMMON_H

    #include <stdint.h>
    #include <stdio.h>

constexpr int OS_AGAIN = 3;

char *os_read_stream(FILE *file, size_t size, char **kept, size_t *len);

bool os_is_within(const char *name, uint32_t depth);

void os_set_loopback(void *address);

int os_get_port(const void *port);

#endif

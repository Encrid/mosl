/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef MOSL_OS_COMMON_H
    #define MOSL_OS_COMMON_H

    #include <stdint.h>
    #include <stdio.h>

    #include "os.h"

constexpr int OS_AGAIN = 3;
constexpr uint32_t OS_FILE_FLAGS = OS_FILE_EXECUTABLE | OS_FILE_DURABLE;
constexpr uint32_t OS_MOVE_FLAGS = OS_MOVE_DURABLE;

char *os_read_stream(FILE *file, size_t size, char **kept, size_t *len);

bool os_is_within(const char *name, uint32_t depth);

void os_set_loopback(void *address);

int os_get_port(const void *port);

#endif

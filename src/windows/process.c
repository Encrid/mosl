/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "os_types.h"

os_process_t *os_process_start([[maybe_unused]] const os_process_start_t *start)
{
    return (nullptr);
}

int os_process_write([[maybe_unused]] os_process_t *p,
    [[maybe_unused]] const void *bytes, [[maybe_unused]] size_t len)
{
    return (-1);
}

void os_process_close([[maybe_unused]] os_process_t *p,
    [[maybe_unused]] int stream)
{
}

int os_process_read([[maybe_unused]] os_process_t *p,
    [[maybe_unused]] uint64_t nanos, [[maybe_unused]] const char **bytes,
    [[maybe_unused]] size_t *len)
{
    return (-1);
}

int os_process_signal([[maybe_unused]] os_process_t *p,
    [[maybe_unused]] int which)
{
    return (-1);
}

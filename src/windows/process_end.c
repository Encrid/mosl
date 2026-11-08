/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "os_types.h"

void os_process_kill([[maybe_unused]] os_process_t *p)
{
}

int os_process_wait([[maybe_unused]] os_process_t *p,
    [[maybe_unused]] uint64_t nanos, [[maybe_unused]] int64_t *status)
{
    return (-1);
}

void os_process_free([[maybe_unused]] os_process_t *p)
{
}

/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <unistd.h>

#include "os_types.h"

uint32_t os_get_online_count(void)
{
    long count = sysconf(_SC_NPROCESSORS_ONLN);

    return (count < 1 ? 0 : (uint32_t)count);
}

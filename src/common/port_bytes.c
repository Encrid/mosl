/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "os_common.h"

constexpr uint8_t LOOPBACK[] = {127, 0, 0, 1};

void os_set_loopback(void *address)
{
    memcpy(address, LOOPBACK, sizeof(LOOPBACK));
}

int os_get_port(const void *port)
{
    const uint8_t *bytes = port;

    return (bytes[0] << 8 | bytes[1]);
}

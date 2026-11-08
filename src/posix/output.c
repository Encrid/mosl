/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <unistd.h>

#include "os_types.h"

int os_output_write(const char *bytes, size_t len)
{
    return (os_write_all(STDOUT_FILENO, -1, bytes, len));
}

/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "../posix/os_types.h"
#include "../common/os_common.h"

char *os_folder_base(uint32_t kind)
{
    if (!os_is_folder_kind(kind))
        return (nullptr);
    return (os_join_base(getenv("HOME"), "/Library/Application Support"));
}

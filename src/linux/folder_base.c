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
    const char *data = getenv("XDG_DATA_HOME");

    if (!os_is_folder_kind(kind))
        return (nullptr);
    if (data && data[0] == '/')
        return (os_join_base(data, ""));
    return (os_join_base(getenv("HOME"), "/.local/share"));
}

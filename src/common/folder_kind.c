/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "os_common.h"

bool os_is_folder_kind(uint32_t kind)
{
    return (kind == OS_FOLDER_USER || kind == OS_FOLDER_LOCAL);
}

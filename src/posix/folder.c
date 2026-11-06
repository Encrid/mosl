/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "os_types.h"

int os_folder_set(const char *folder)
{
    return (chdir(folder));
}

int os_folder_make(const char *folder)
{
    const char *name = nullptr;
    int at = os_open_parent(folder, &name, true);
    struct stat there = {};
    int made = -1;

    if (at < 0)
        return (-1);
    made = mkdirat(at, name, 0777);
    if (made && errno == EEXIST && !fstatat(at, name, &there, 0))
        made = S_ISDIR(there.st_mode) ? 0 : -1;
    os_close_parent(at);
    return (made ? -1 : 0);
}

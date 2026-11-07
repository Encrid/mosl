/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>

#include "os_types.h"

int os_folder_set(const char *folder)
{
    return (chdir(folder));
}

static bool is_folder_there(const char *folder, int at, const char *name)
{
    struct stat there = {};

    if (strlen(folder) < PATH_MAX)
        return (!fstatat(AT_FDCWD, folder, &there, 0)
            && S_ISDIR(there.st_mode));
    return (os_open_search(at, name) >= 0);
}

int os_folder_make(const char *folder)
{
    const char *name = nullptr;
    int at = os_open_parent(folder, &name, true);
    int made = at < 0 ? -1 : -2;

    if (at >= 0) {
        if (strlen(folder) < PATH_MAX)
            made = mkdirat(AT_FDCWD, folder, 0777);
        else
            made = mkdirat(at, name, 0777);
        if (made && errno == EEXIST)
            made = is_folder_there(folder, at, name) ? 0 : -1;
    }
    os_close_parent(at);
    return (made ? -1 : 0);
}

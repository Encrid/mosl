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
    int made = -1;
    int there = -1;

    if (at >= 0) {
        made = mkdirat(at, name, 0777);
        if (made && errno == EEXIST) {
            there = os_open_search(at, name);
            made = there < 0;
        }
        os_close_parent(there);
    }
    os_close_parent(at);
    return (made ? -1 : 0);
}

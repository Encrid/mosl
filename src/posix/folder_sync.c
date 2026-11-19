/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

#include "os_types.h"

constexpr int READABLE = O_RDONLY | O_DIRECTORY | O_CLOEXEC;

int os_sync_folder(int at, const char *name)
{
    char folder[PATH_MAX] = ".";
    size_t last = os_get_last_start(name);
    int fd = -1;
    int synced = -1;

    if (last) {
        memcpy(folder, name, last);
        folder[last] = '\0';
    }
    fd = openat(at, folder, READABLE);
    if (fd < 0)
        return (-1);
    synced = fsync(fd);
    close(fd);
    return (synced ? -1 : 0);
}

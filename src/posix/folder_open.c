/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "os_types.h"

constexpr int FOLDER = O_RDONLY | O_DIRECTORY | O_CLOEXEC;

os_folder_t *os_folder_open(const char *path)
{
    os_folder_t *f = calloc(1, sizeof(*f));
    const char *name = nullptr;
    int at = -1;
    int fd = -1;

    if (!f)
        return (nullptr);
    f->dir = opendir(path);
    if (!f->dir && errno == ENAMETOOLONG)
        at = os_open_parent(path, &name, false);
    if (at >= 0)
        fd = openat(at, name, FOLDER);
    os_close_parent(at);
    if (fd >= 0)
        f->dir = fdopendir(fd);
    if (f->dir)
        return (f);
    close(fd);
    free(f);
    return (nullptr);
}

void os_folder_close(os_folder_t *f)
{
    if (!f)
        return;
    closedir(f->dir);
    free(f->bytes);
    free(f);
}

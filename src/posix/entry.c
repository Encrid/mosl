/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

#include "os_types.h"

int os_path_entry(const char *path)
{
    const char *name = nullptr;
    int at = os_open_parent(path, &name, false);
    struct stat st = {};
    int found = at == -1 ? -1 : fstatat(at, name, &st, 0);
    int kind = -1;

    if (!found)
        kind = S_ISDIR(st.st_mode) ? 3 : 1 + ((st.st_mode & 0111) != 0);
    else if (errno == ENOENT || errno == ENOTDIR)
        kind = 0;
    os_close_parent(at);
    return (kind);
}

int os_path_move(const char *from, const char *to)
{
    const char *from_name = nullptr;
    const char *to_name = nullptr;
    int from_at = os_open_parent(from, &from_name, false);
    int to_at = os_open_parent(to, &to_name, false);
    int moved = -1;

    if (from_at != -1 && to_at != -1)
        moved = renameat(from_at, from_name, to_at, to_name);
    os_close_parent(from_at);
    os_close_parent(to_at);
    return (moved ? -1 : 0);
}

int os_link_make(const char *path, const char *target)
{
    const char *name = nullptr;
    int at = os_open_parent(path, &name, false);
    int linked = at == -1 ? -1 : symlinkat(target, at, name);

    os_close_parent(at);
    return (linked ? -1 : 0);
}

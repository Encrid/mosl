/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "os_types.h"

constexpr int FOLDER = O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC;

static DIR *open_writable(int at, const char *name, mode_t mode)
{
    int fd = -1;
    DIR *dir = nullptr;

    if ((mode & S_IRWXU) != S_IRWXU
        && fchmodat(at, name, (mode & 07777) | S_IRWXU, 0))
        return (nullptr);
    fd = openat(at, name, FOLDER);
    dir = fd < 0 ? nullptr : fdopendir(fd);
    if (!dir && fd >= 0)
        close(fd);
    return (dir);
}

static int remove_entries(int at, const char *name, mode_t mode)
{
    DIR *dir = open_writable(at, name, mode);
    struct dirent *entry = dir ? readdir(dir) : nullptr;
    int removed = dir ? 0 : -1;

    while (!removed && entry) {
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
            removed = os_remove_entry(dirfd(dir), entry->d_name);
        entry = readdir(dir);
    }
    if (dir)
        closedir(dir);
    return (removed);
}

int os_remove_entry(int at, const char *name)
{
    struct stat st = {};

    if (fstatat(at, name, &st, AT_SYMLINK_NOFOLLOW))
        return (-1);
    if (!S_ISDIR(st.st_mode))
        return (unlinkat(at, name, 0) ? -1 : 0);
    if (remove_entries(at, name, st.st_mode))
        return (-1);
    return (unlinkat(at, name, AT_REMOVEDIR) ? -1 : 0);
}

int os_path_remove(const char *path)
{
    const char *name = nullptr;
    int at = os_open_parent(path, &name, false);
    struct stat st = {};
    int found = at == -1 ? -1 : fstatat(at, name, &st, AT_SYMLINK_NOFOLLOW);
    int removed = -1;

    if (!found)
        removed = os_remove_entry(at, name);
    else if (errno == ENOENT || errno == ENOTDIR)
        removed = 1;
    os_close_parent(at);
    return (removed);
}

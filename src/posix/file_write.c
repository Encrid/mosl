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

constexpr int WRITE = O_WRONLY | O_CLOEXEC;

static mode_t get_kept_mode(mode_t mode, bool executable)
{
    mode_t kept = mode & 0666;

    if (executable)
        kept |= (mode & 0444) >> 2;
    return (kept);
}

static int open_file(int at, const char *name, bool executable)
{
    int fd = openat(at, name, WRITE | O_CREAT | O_EXCL,
        executable ? 0777 : 0666);
    struct stat st = {};
    mode_t kept = 0;

    if (fd >= 0 || errno != EEXIST)
        return (fd);
    fd = openat(at, name, WRITE | O_TRUNC);
    if (fd < 0)
        return (-1);
    if (!fstat(fd, &st)) {
        kept = get_kept_mode(st.st_mode, executable);
        if (kept == (st.st_mode & 07777) || !fchmod(fd, kept))
            return (fd);
    }
    close(fd);
    return (-1);
}

int os_file_write(const char *path, const void *bytes, size_t len,
    bool executable)
{
    const char *name = nullptr;
    int at = os_open_parent(path, &name, false);
    int fd = at == -1 ? -1 : open_file(at, name, executable);
    int wrote = fd < 0 ? -1 : os_write_all(fd, -1, bytes, len);

    os_close_parent(at);
    if (fd >= 0 && close(fd))
        wrote = -1;
    return (wrote);
}

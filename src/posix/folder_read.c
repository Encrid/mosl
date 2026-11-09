/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "os_types.h"
#include "../common/os_common.h"

const char *os_folder_read_entry(os_folder_t *f, bool *is_folder)
{
    struct dirent *e = readdir(f->dir);
    struct stat st = {};

    while (e && (!strcmp(e->d_name, ".") || !strcmp(e->d_name, "..")))
        e = readdir(f->dir);
    if (!e)
        return (nullptr);
    fstatat(dirfd(f->dir), e->d_name, &st, 0);
    *is_folder = S_ISDIR(st.st_mode);
    return (e->d_name);
}

char *os_folder_read_file(os_folder_t *f, const char *name, size_t *len)
{
    int fd = openat(dirfd(f->dir), name, O_RDONLY | O_CLOEXEC);
    struct stat st = {};
    FILE *file = nullptr;
    char *bytes = nullptr;

    if (fd < 0)
        return (nullptr);
    fstat(fd, &st);
    if (S_ISREG(st.st_mode))
        file = fdopen(fd, "rb");
    if (!file) {
        close(fd);
        return (nullptr);
    }
    bytes = os_read_stream(file, (size_t)st.st_size, &f->bytes, len);
    fclose(file);
    return (bytes);
}

int os_folder_entry_size(os_folder_t *f, const char *name, uint64_t *size)
{
    struct stat st = {};

    if (fstatat(dirfd(f->dir), name, &st, 0) || !S_ISREG(st.st_mode))
        return (-1);
    *size = (uint64_t)st.st_size;
    return (0);
}

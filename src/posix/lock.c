/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/file.h>
#include <unistd.h>

#include "../common/os_common.h"

int os_lock(const char *path, os_lock_t **lock)
{
    os_lock_t *taken = malloc(sizeof(*taken));
    bool opened = false;
    int held = -1;

    *lock = nullptr;
    if (taken)
        taken->file = open(path, O_RDWR | O_CREAT | O_CLOEXEC, 0644);
    opened = taken && taken->file >= 0;
    if (opened && flock(taken->file, LOCK_EX | LOCK_NB)) {
        held = errno == EWOULDBLOCK ? 1 : -1;
        close(taken->file);
    } else if (opened)
        held = 0;
    if (held)
        free(taken);
    else
        *lock = taken;
    return (held);
}

void os_unlock(os_lock_t *lock)
{
    if (!lock)
        return;
    flock(lock->file, LOCK_UN);
    close(lock->file);
    free(lock);
}

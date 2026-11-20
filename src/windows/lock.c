/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <fcntl.h>
#include <io.h>
#include <stdlib.h>
#include <sys/locking.h>
#include <sys/stat.h>

#include "../common/os_common.h"

int os_lock(const char *path, os_lock_t **lock)
{
    os_lock_t *taken = malloc(sizeof(*taken));
    int held = -1;

    *lock = nullptr;
    if (taken)
        taken->file = _open(path, _O_RDWR | _O_CREAT, _S_IREAD | _S_IWRITE);
    if (taken && taken->file >= 0)
        held = _locking(taken->file, _LK_NBLCK, 1) ? 1 : 0;
    if (held == 1)
        _close(taken->file);
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
    _locking(lock->file, _LK_UNLCK, 1);
    _close(lock->file);
    free(lock);
}

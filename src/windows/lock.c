/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <stdlib.h>
#include <sys/locking.h>
#include <sys/stat.h>

#include "os_types.h"

int os_lock(const char *path, os_lock_t **lock)
{
    os_lock_t *taken = malloc(sizeof(*taken));
    bool opened = false;
    int held = -1;

    *lock = nullptr;
    if (taken)
        taken->file = _open(path, _O_RDWR | _O_CREAT | _O_NOINHERIT,
            _S_IREAD | _S_IWRITE);
    opened = taken && taken->file >= 0;
    if (opened && _locking(taken->file, _LK_NBLCK, 1)) {
        held = _doserrno == ERROR_LOCK_VIOLATION ? 1 : -1;
        _close(taken->file);
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
    _locking(lock->file, _LK_UNLCK, 1);
    _close(lock->file);
    free(lock);
}

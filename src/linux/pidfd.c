/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <sys/syscall.h>
#include <unistd.h>

#include "../posix/os_types.h"

int os_pidfd_open(pid_t pid)
{
    return ((int)syscall(SYS_pidfd_open, pid, 0));
}

/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <unistd.h>

#include "os_types.h"

int os_process_write(os_process_t *p, const void *bytes, size_t len)
{
    if (p->input < 0)
        return (-1);
    return (os_write_all(p->input, p->wake[0], bytes, len));
}

void os_process_close(os_process_t *p, int stream)
{
    int *ends[] = {&p->input, &p->input, &p->output, &p->errors};

    close(*ends[stream]);
    *ends[stream] = -1;
}

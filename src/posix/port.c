/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "os.h"
#include "../common/os_common.h"

int os_get_free_port(void)
{
    struct sockaddr_in at = {};
    socklen_t size = sizeof(at);
    int s = socket(AF_INET, SOCK_STREAM, 0);
    int port = -1;

    at.sin_family = AF_INET;
    os_set_loopback(&at.sin_addr);
    if (s >= 0 && !bind(s, (struct sockaddr *)&at, sizeof(at))
        && !getsockname(s, (struct sockaddr *)&at, &size))
        port = os_get_port(&at.sin_port);
    if (s >= 0)
        close(s);
    return (port);
}

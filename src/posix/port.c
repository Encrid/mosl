/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "os.h"

int os_get_free_port(void)
{
    struct sockaddr_in at = {};
    socklen_t size = sizeof(at);
    int s = socket(AF_INET, SOCK_STREAM, 0);
    int port = -1;

    at.sin_family = AF_INET;
    at.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (s >= 0 && !bind(s, (struct sockaddr *)&at, sizeof(at))
        && !getsockname(s, (struct sockaddr *)&at, &size))
        port = ntohs(at.sin_port);
    if (s >= 0)
        close(s);
    return (port);
}

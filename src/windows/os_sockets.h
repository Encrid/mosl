/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef MOSL_OS_SOCKETS_H
    #define MOSL_OS_SOCKETS_H

    #include <winsock2.h>

    #include "os_types.h"

typedef int (WSAAPI *startup_t)(WORD version, LPWSADATA data);
typedef SOCKET (WSAAPI *socket_t)(int family, int type, int protocol);
typedef int (WSAAPI *bind_t)(SOCKET s, const struct sockaddr *at, int size);
typedef int (WSAAPI *name_t)(SOCKET s, struct sockaddr *at, int *size);
typedef int (WSAAPI *close_t)(SOCKET s);
typedef int (WSAAPI *cleanup_t)(void);

typedef struct {
    startup_t startup;
    socket_t socket;
    bind_t bind;
    name_t name;
    close_t close;
    cleanup_t cleanup;
} sockets_t;

#endif

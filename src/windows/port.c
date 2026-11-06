/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "os_sockets.h"

static const char *const NAMES[] = {
    "WSAStartup", "socket", "bind", "getsockname", "closesocket", "WSACleanup"
};
constexpr size_t COUNT = sizeof(NAMES) / sizeof(NAMES[0]);

constexpr uint8_t LOOPBACK[] = {127, 0, 0, 1};

static bool load_calls(HMODULE library, sockets_t *calls)
{
    void *found[COUNT] = {};

    for (size_t i = 0; i < COUNT; i++) {
        found[i] = (void *)GetProcAddress(library, NAMES[i]);
        if (!found[i])
            return (false);
    }
    calls->startup = (startup_t)found[0];
    calls->socket = (socket_t)found[1];
    calls->bind = (bind_t)found[2];
    calls->name = (name_t)found[3];
    calls->close = (close_t)found[4];
    calls->cleanup = (cleanup_t)found[5];
    return (true);
}

static int get_port(const sockets_t *calls)
{
    struct sockaddr_in at = {};
    int size = sizeof(at);
    SOCKET s = calls->socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    const uint8_t *bytes = (const uint8_t *)&at.sin_port;
    int port = -1;

    at.sin_family = AF_INET;
    memcpy(&at.sin_addr, LOOPBACK, sizeof(LOOPBACK));
    if (s != INVALID_SOCKET && !calls->bind(s, (struct sockaddr *)&at,
        sizeof(at)) && !calls->name(s, (struct sockaddr *)&at, &size))
        port = bytes[0] << 8 | bytes[1];
    if (s != INVALID_SOCKET)
        calls->close(s);
    return (port);
}

int os_get_free_port(void)
{
    HMODULE library = LoadLibraryW(L"ws2_32.dll");
    sockets_t calls = {};
    WSADATA data = {};
    int port = -1;

    if (library && load_calls(library, &calls)
        && !calls.startup(MAKEWORD(2, 2), &data)) {
        port = get_port(&calls);
        calls.cleanup();
    }
    if (library)
        FreeLibrary(library);
    return (port);
}

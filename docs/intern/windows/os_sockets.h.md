# src/windows/os_sockets.h

This header gives the types of the calls of `ws2_32.dll` that `src/windows/port.c` loads.
It includes `<winsock2.h>` and `src/windows/os_types.h`.

## Types

| Type | Content |
| --- | --- |
| `startup_t`, `socket_t`, `bind_t`, `name_t`, `close_t`, `cleanup_t` | The types of `WSAStartup`, `socket`, `bind`, `getsockname`, `closesocket` and `WSACleanup`. |
| `sockets_t` | A structure with one field of each type: `startup`, `socket`, `bind`, `name`, `close` and `cleanup`. |

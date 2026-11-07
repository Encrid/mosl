/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "os_types.h"

constexpr DWORD ENDING[] = {
    CTRL_CLOSE_EVENT, CTRL_LOGOFF_EVENT, CTRL_SHUTDOWN_EVENT
};

static struct os_stop *get_stop(void)
{
    static struct os_stop stop;

    return (&stop);
}

static DWORD WINAPI write_line(LPVOID p)
{
    const struct os_stop *stop = p;
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD written;

    WriteFile(output, stop->line, stop->length, &written, nullptr);
    return (0);
}

static BOOL WINAPI catch_stop(DWORD event)
{
    struct os_stop *stop = get_stop();

    if (atomic_exchange_explicit(&stop->caught, true, memory_order_seq_cst))
        _Exit(1);
    CloseHandle(CreateThread(nullptr, 0, write_line, stop, 0, nullptr));
    for (size_t i = 0; i < sizeof(ENDING) / sizeof(ENDING[0]); i++) {
        if (ENDING[i] == event)
            Sleep(INFINITE);
    }
    return (TRUE);
}

int os_signal_catch(const char *line)
{
    struct os_stop *stop = get_stop();

    stop->line = line;
    stop->length = (DWORD)strlen(line);
    return (!SetConsoleCtrlHandler(catch_stop, TRUE));
}

bool os_signal_is_caught(void)
{
    return (atomic_load_explicit(&get_stop()->caught, memory_order_seq_cst));
}

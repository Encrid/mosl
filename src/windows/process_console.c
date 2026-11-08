/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "os_types.h"

constexpr char INTERRUPT[] = "\x03";
constexpr char BREAK[] = "\x1b[3;70;0;1;8;1_";
constexpr COORD CELLS = {80, 25};
constexpr DWORD GRACE = 1000;
constexpr UINT HUNG = 0xC0000142;

int os_make_console(os_process_t *p)
{
    HANDLE read = nullptr;
    HANDLE nul = CreateFileW(L"NUL", GENERIC_WRITE, FILE_SHARE_WRITE, nullptr,
        OPEN_EXISTING, 0, nullptr);
    HRESULT made = E_FAIL;

    if (nul != INVALID_HANDLE_VALUE
        && CreatePipe(&read, &p->console_input, nullptr, 0))
        made = CreatePseudoConsole(CELLS, read, nul, 0, &p->console);
    if (read)
        CloseHandle(read);
    if (nul != INVALID_HANDLE_VALUE)
        CloseHandle(nul);
    return (made == S_OK ? 0 : -1);
}

static int write_key(os_process_t *p, const char *key, DWORD len)
{
    DWORD wrote = 0;

    return (WriteFile(p->console_input, key, len, &wrote, nullptr) ? 0 : -1);
}

int os_process_signal(os_process_t *p, int which)
{
    if (!p->signals || p->hung)
        return (-1);
    if (which == OS_INTERRUPT)
        return (write_key(p, INTERRUPT, sizeof(INTERRUPT) - 1));
    if (which == OS_TERMINATION)
        return (write_key(p, BREAK, sizeof(BREAK) - 1));
    ClosePseudoConsole(p->console);
    p->console = nullptr;
    p->hung = true;
    WaitForSingleObject(p->process, GRACE);
    TerminateProcess(p->process, HUNG);
    return (0);
}

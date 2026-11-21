/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <shlobj.h>

#include "os_types.h"
#include "../common/os_common.h"

static const GUID SAVED_GAMES = {
    0x4c5c32ff, 0xbb9d, 0x43b0, {0xb5, 0xb4, 0x2d, 0x72, 0xe5, 0x4e, 0xaa, 0xa4}
};
static const GUID LOCAL_APP_DATA = {
    0xf1b32785, 0x6fba, 0x4fcf, {0x9d, 0x55, 0x7b, 0x8e, 0x7f, 0x15, 0x70, 0x91}
};

static char *get_known(HMODULE shell, HMODULE ole, const GUID *id)
{
    os_known_path_t known = (os_known_path_t)(void *)GetProcAddress(shell,
        "SHGetKnownFolderPath");
    os_task_free_t task_free = (os_task_free_t)(void *)GetProcAddress(ole,
        "CoTaskMemFree");
    wchar_t *wide = nullptr;
    char *path = nullptr;

    if (known && task_free && known(id, KF_FLAG_CREATE, nullptr, &wide) == S_OK)
        path = os_narrow(wide);
    if (task_free)
        task_free(wide);
    return (path);
}

char *os_folder_base(uint32_t kind)
{
    const GUID *id = kind == OS_FOLDER_USER ? &SAVED_GAMES : &LOCAL_APP_DATA;
    HMODULE shell = os_is_folder_kind(kind) ? LoadLibraryW(L"shell32.dll")
        : nullptr;
    HMODULE ole = shell ? LoadLibraryW(L"ole32.dll") : nullptr;
    char *path = ole ? get_known(shell, ole, id) : nullptr;

    if (ole)
        FreeLibrary(ole);
    if (shell)
        FreeLibrary(shell);
    return (path);
}

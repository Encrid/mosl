/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef MOSL_OS_TYPES_H
    #define MOSL_OS_TYPES_H

    #define WIN32_LEAN_AND_MEAN
    #include <stdatomic.h>
    #include <windows.h>

    #include "os.h"

struct os_thread {
    HANDLE handle;
    os_thread_fn_t fn;
    void *arg;
};

struct os_mutex {
    CRITICAL_SECTION section;
};

struct os_event {
    HANDLE handle;
};

constexpr size_t OS_NAME_ROOM = 256;

struct os_folder {
    HANDLE find;
    WIN32_FIND_DATAW data;
    bool pending;
    wchar_t *whole;
    char *bytes;
    char name[3 * OS_NAME_ROOM];
};

constexpr DWORD OS_CHANGES_ROOM = 65536;
constexpr DWORD OS_ABOVE_ROOM = 1024;
constexpr size_t OS_NAME_BYTES = 3 * OS_CHANGES_ROOM / 2 + 1;

struct os_watch {
    HANDLE folder;
    HANDLE above;
    OVERLAPPED changes;
    OVERLAPPED above_changes;
    wchar_t *final;
    wchar_t *again;
    uint32_t depth;
    DWORD at;
    DWORD got;
    bool reading;
    bool looking;
    bool stopped;
    alignas(DWORD) BYTE records[OS_CHANGES_ROOM];
    alignas(DWORD) BYTE above_records[OS_ABOVE_ROOM];
    char name[OS_NAME_BYTES];
};

struct os_stop {
    atomic_bool caught;
    const char *line;
    DWORD length;
};

bool os_read_changes(os_watch_t *w);

bool os_read_above(os_watch_t *w);

bool os_is_in_place(os_watch_t *w);

int os_finish_read(os_watch_t *w);

void os_end_read(HANDLE handle, OVERLAPPED *overlapped);

wchar_t *os_widen(const char *text);

char *os_narrow(const wchar_t *wide);

int os_write_all(HANDLE handle, const void *bytes, size_t len);

char *os_whole_text(const char *path);

char *os_get_variable(const char *name);

int os_remove_entry(const wchar_t *whole);

#endif

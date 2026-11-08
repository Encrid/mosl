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

constexpr size_t OS_PROCESS_BYTES = 65536;

typedef struct {
    HANDLE pipe;
    OVERLAPPED read;
    bool pending;
    char bytes[OS_PROCESS_BYTES];
} os_stream_t;

struct os_process {
    HANDLE process;
    HANDLE input;
    HANDLE stop;
    HANDLE writing;
    HANDLE console_input;
    HPCON console;
    bool signals;
    bool hung;
    unsigned turn;
    os_stream_t streams[2];
};

typedef struct {
    wchar_t *program;
    wchar_t *line;
    wchar_t *block;
    wchar_t *folder;
    HANDLE child[3];
} os_launch_t;

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

wchar_t **os_widen_texts(const char *const *texts);

void os_free_texts(wchar_t **texts);

int os_write_all(HANDLE handle, const void *bytes, size_t len);

char *os_whole_text(const char *path);

char *os_get_variable(const char *name);

int os_remove_entry(const wchar_t *whole);

char *os_find_text(const char *name);

wchar_t *os_find_program(const os_process_start_t *start);

bool os_prepare_launch(const os_process_start_t *start, os_launch_t *launch);

wchar_t *os_join_line(const char *const *argv);

wchar_t *os_build_block(const char *const *env, const char *const *unset);

int os_open_pipe(os_process_t *p, int index, bool inbound, HANDLE ends[2]);

int os_make_console(os_process_t *p);

DWORD os_get_wait(uint64_t deadline);

void os_close_stream(os_stream_t *s);

void os_drop_process(os_process_t *p);

#endif

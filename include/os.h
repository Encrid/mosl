/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef MOSL_OS_H
    #define MOSL_OS_H

    #include <stddef.h>
    #include <stdint.h>

typedef struct os_thread os_thread_t;
typedef struct os_mutex os_mutex_t;
typedef struct os_event os_event_t;
typedef struct os_folder os_folder_t;
typedef struct os_watch os_watch_t;
typedef struct os_process os_process_t;
typedef void (*os_thread_fn_t)(void *arg);

typedef struct os_process_start {
    const char *const *argv;
    const char *folder;
    const char *const *env;
    const char *const *unset;
    bool errors_apart;
    bool signals;
    bool ignore_interrupt;
} os_process_start_t;

constexpr uint64_t OS_SECOND = 1'000'000'000;
constexpr uint64_t OS_FOREVER = UINT64_MAX;
constexpr int OS_INPUT = 1;
constexpr int OS_OUTPUT = 2;
constexpr int OS_ERRORS = 3;
constexpr int OS_INTERRUPT = 1;
constexpr int OS_TERMINATION = 2;
constexpr int OS_HANG_UP = 3;

[[nodiscard]] void *os_reserve(size_t reserve, size_t commit);

[[nodiscard]] int os_commit(void *base, size_t bytes);

void os_release(void *base, size_t reserve);

[[nodiscard]] void *os_lib_open(const char *path);

[[nodiscard]] void *os_lib_get_symbol(void *lib, const char *name);

void os_lib_close(void *lib);

[[nodiscard]] int os_lib_pin(const char *path);

[[nodiscard]] os_thread_t *os_thread_start(os_thread_fn_t fn, void *arg,
    size_t stack);

void os_thread_join(os_thread_t *t);

[[nodiscard]] size_t os_thread_stack_left(void);

uint64_t os_get_thread_id(void);

uint64_t os_get_now(void);

[[nodiscard]] os_mutex_t *os_mutex_create(void);

void os_mutex_free(os_mutex_t *m);

void os_mutex_lock(os_mutex_t *m);

void os_mutex_unlock(os_mutex_t *m);

[[nodiscard]] os_event_t *os_event_create(void);

void os_event_free(os_event_t *e);

void os_event_set(os_event_t *e);

void os_event_wait(os_event_t *e, uint64_t nanos);

[[nodiscard]] int os_folder_set(const char *folder);

[[nodiscard]] int os_folder_make(const char *folder);

[[nodiscard]] os_folder_t *os_folder_open(const char *path);

[[nodiscard]] const char *os_folder_read_entry(os_folder_t *f, bool *is_folder);

[[nodiscard]] char *os_folder_read_file(os_folder_t *f, const char *name,
    size_t *len);

[[nodiscard]] int os_folder_entry_size(os_folder_t *f, const char *name,
    uint64_t *size);

void os_folder_close(os_folder_t *f);

[[nodiscard]] os_watch_t *os_watch_start(const char *folder, uint32_t depth);

[[nodiscard]] int os_watch_take(os_watch_t *w, const char **name);

void os_watch_stop(os_watch_t *w);

[[nodiscard]] char *os_path_whole(const char *path);

[[nodiscard]] int os_lock(const char *path);

[[nodiscard]] int os_signal_catch(const char *line);

bool os_signal_is_caught(void);

[[nodiscard]] os_process_t *os_process_start(const os_process_start_t *start);

[[nodiscard]] int os_process_write(os_process_t *p, const void *bytes,
    size_t len);

void os_process_close(os_process_t *p, int stream);

[[nodiscard]] int os_process_read(os_process_t *p, uint64_t nanos,
    const char **bytes, size_t *len);

[[nodiscard]] int os_process_signal(os_process_t *p, int which);

void os_process_kill(os_process_t *p);

[[nodiscard]] int os_process_wait(os_process_t *p, uint64_t nanos,
    int64_t *status);

void os_process_free(os_process_t *p);

[[nodiscard]] int os_file_write(const char *path, const void *bytes,
    size_t len, bool executable);

[[nodiscard]] int os_path_entry(const char *path);

[[nodiscard]] int os_path_move(const char *from, const char *to);

[[nodiscard]] int os_path_remove(const char *path);

[[nodiscard]] int os_link_make(const char *path, const char *target);

[[nodiscard]] char *os_env_get(const char *name);

[[nodiscard]] char *os_path_find(const char *name);

[[nodiscard]] char *os_get_program_path(void);

[[nodiscard]] int os_output_write(const char *bytes, size_t len);

[[nodiscard]] int os_get_free_port(void);

[[nodiscard]] uint32_t os_get_processor_count(void);

#endif

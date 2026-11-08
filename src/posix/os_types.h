/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef MOSL_OS_TYPES_H
    #define MOSL_OS_TYPES_H

    #include <dirent.h>
    #include <pthread.h>
    #include <signal.h>
    #include <stdatomic.h>

    #include "os.h"

struct os_thread {
    pthread_t handle;
    os_thread_fn_t fn;
    void *arg;
};

struct os_mutex {
    pthread_mutex_t mutex;
};

struct os_event {
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int set;
};

struct os_folder {
    DIR *dir;
    char *bytes;
};

constexpr int OS_PIPE_INPUT = 0;
constexpr int OS_PIPE_OUTPUT = 1;
constexpr int OS_PIPE_ERRORS = 2;
constexpr int OS_PIPE_GATE = 3;
constexpr int OS_PIPE_REPORT = 4;
constexpr int OS_PIPES = 5;
constexpr size_t OS_PROCESS_BYTES = 65536;

struct os_process {
    pid_t pid;
    int input;
    int output;
    int errors;
    int wake[2];
    int exit;
    bool signals;
    bool hung;
    bool taken;
    int64_t status;
    unsigned turn;
    pthread_mutex_t lock;
    char bytes[OS_PROCESS_BYTES];
};

typedef struct {
    char *path;
    char **envp;
    int pipes[OS_PIPES][2];
} os_launch_t;

struct os_stop {
    atomic_bool caught;
    atomic_uintptr_t line;
    atomic_size_t length;
};

int os_cond_start(pthread_cond_t *cond);

void os_cond_wait(os_event_t *e, uint64_t nanos);

int os_open_search(int at, const char *name);

int os_open_parent(const char *path, const char **name, bool make);

void os_close_parent(int at);

int os_write_all(int fd, int wake, const void *bytes, size_t len);

bool os_is_pipe_waiting(void);

void os_take_pipe_signal(const sigset_t *pipe_only);

int os_open_pipe(int ends[2]);

int os_watch_exit(os_process_t *p);

int os_wait_exit(os_process_t *p, uint64_t deadline);

int os_pidfd_open(pid_t pid);

int os_get_wait(uint64_t deadline);

char *os_find_program(const char *name);

char **os_build_env(const char *const *env, const char *const *unset);

[[noreturn]] void os_run_child(const os_process_start_t *start,
    const os_launch_t *launch);

void os_close_pipes(int pipes[][2], int count);

void os_drop_process(os_process_t *p);

uint32_t os_get_online_count(void);

int os_remove_entry(int at, const char *name);

#endif

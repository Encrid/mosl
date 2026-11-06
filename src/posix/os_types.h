/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef MOSL_OS_TYPES_H
    #define MOSL_OS_TYPES_H

    #include <dirent.h>
    #include <pthread.h>
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

struct os_stop {
    atomic_bool caught;
    atomic_uintptr_t line;
    atomic_size_t length;
};

int os_cond_start(pthread_cond_t *cond);

void os_cond_wait(os_event_t *e, uint64_t nanos);

int os_open_search(int at, const char *name);

#endif

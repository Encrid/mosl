/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "os.h"
#include "rows.h"

constexpr size_t STACK = 1 << 16;
constexpr uint64_t HELD = OS_SECOND / 10;

typedef struct {
    os_mutex_t *mutex;
    os_event_t *trying;
    os_event_t *taken;
    int count;
} shared_t;

static void take(void *arg)
{
    shared_t *shared = arg;

    os_event_set(shared->trying);
    os_mutex_lock(shared->mutex);
    shared->count++;
    os_mutex_unlock(shared->mutex);
    os_event_set(shared->taken);
}

static const char *get_seen(shared_t *shared)
{
    const char *seen;

    os_mutex_lock(shared->mutex);
    seen = shared->count ? "taken" : "waited";
    os_mutex_unlock(shared->mutex);
    return (seen);
}

static void take_after(shared_t *shared, bool held)
{
    os_thread_t *t = os_thread_start(take, shared, STACK);
    const char *first = "no thread";

    if (t && held)
        os_event_wait(shared->trying, OS_FOREVER);
    if (t) {
        os_event_wait(shared->taken, held ? HELD : OS_FOREVER);
        first = shared->count ? "taken" : "waited";
    }
    if (held)
        os_mutex_unlock(shared->mutex);
    if (!t) {
        printf("mutex: %s\n", first);
        return;
    }
    os_thread_join(t);
    printf("mutex: %s, then %s\n", first, get_seen(shared));
}

static int wait_mutex(bool held)
{
    shared_t shared = {os_mutex_create(), os_event_create(),
        os_event_create(), 0};

    if (shared.mutex && shared.trying && shared.taken) {
        if (held)
            os_mutex_lock(shared.mutex);
        take_after(&shared, held);
    } else
        printf("mutex: not made\n");
    os_event_free(shared.taken);
    os_event_free(shared.trying);
    os_mutex_free(shared.mutex);
    return (0);
}

static void read_id(void *arg)
{
    *(uint64_t *)arg = os_get_thread_id();
}

static int compare_ids(bool other)
{
    uint64_t first = os_get_thread_id();
    uint64_t second = 0;
    os_thread_t *t = nullptr;

    if (other)
        t = os_thread_start(read_id, &second, STACK);
    if (t)
        os_thread_join(t);
    else
        read_id(&second);
    printf("ids: %s, %s\n", first == second ? "same" : "differ",
        first && second ? "none 0" : "a 0");
    return (0);
}

static int run(int argc, char **argv)
{
    if (argc == 2 && !strcmp(argv[1], "held"))
        return (wait_mutex(true));
    if (argc == 2 && !strcmp(argv[1], "free"))
        return (wait_mutex(false));
    if (argc == 3 && !strcmp(argv[1], "ids"))
        return (compare_ids(!strcmp(argv[2], "thread")));
    return (printf("usage: threads held|free|ids thread|main\n"), 2);
}

int main(int argc, char **argv)
{
    return (argc > 1 ? run(argc, argv) : rows_run(run));
}

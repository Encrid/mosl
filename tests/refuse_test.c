/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "os.h"
#include "rows.h"

constexpr unsigned TRIES = 8;

static void stay([[maybe_unused]] void *arg)
{
}

static bool make_event(void)
{
    os_event_t *e = os_event_create();
    bool made = e != nullptr;

    os_event_free(e);
    return (made);
}

static bool make_mutex(void)
{
    os_mutex_t *m = os_mutex_create();
    bool made = m != nullptr;

    os_mutex_free(m);
    return (made);
}

static bool make_thread(void)
{
    os_thread_t *t = os_thread_start(stay, nullptr, 1 << 16);
    bool made = t != nullptr;

    if (made)
        os_thread_join(t);
    return (made);
}

static int run(int argc, char **argv)
{
    bool (*make)(void) = nullptr;
    unsigned refused = 0;

    if (argc == 2 && !strcmp(argv[1], "event"))
        make = make_event;
    if (argc == 2 && !strcmp(argv[1], "mutex"))
        make = make_mutex;
    if (argc == 2 && !strcmp(argv[1], "thread"))
        make = make_thread;
    if (!make)
        return (printf("usage: refuse event|mutex|thread\n"), 2);
    while (refused < TRIES && !make())
        refused++;
    if (!refused)
        printf("%s: made at once\n", argv[1]);
    else if (refused < TRIES)
        printf("%s: %u refused, then made\n", argv[1], refused);
    else
        printf("%s: always refused\n", argv[1]);
    return (0);
}

int main(void)
{
    return (rows_run(run));
}

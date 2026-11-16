/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef _WIN32
    #include <dlfcn.h>
    #include <time.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "os.h"
#include "rows.h"

static long long step;

#ifndef _WIN32
int clock_gettime(clockid_t clock, struct timespec *now)
{
    void *found = dlsym(RTLD_NEXT, "clock_gettime");
    int (*read_clock)(clockid_t, struct timespec *);
    int result;

    memcpy(&read_clock, &found, sizeof(read_clock));
    result = read_clock(clock, now);
    if (clock == CLOCK_REALTIME)
        now->tv_sec -= step;
    return (result);
}
#endif

static unsigned count_short(os_event_t *e, uint64_t nanos, unsigned waits)
{
    unsigned short_waits = 0;
    uint64_t spent;

    for (unsigned i = 0; i < waits; i++) {
        spent = os_get_now();
        os_event_wait(e, nanos);
        short_waits += os_get_now() - spent < nanos;
    }
    return (short_waits);
}

static int run(int argc, char **argv)
{
    os_event_t *e = os_event_create();
    uint64_t nanos;
    unsigned waits;
    unsigned short_waits;

    if (argc < 4 || argc > 5 || !e)
        return (printf("usage: wait <nanoseconds> <seconds back> <waits> [set|taken]\n"), 2);
    nanos = strtoull(argv[1], nullptr, 10);
    step = strtoll(argv[2], nullptr, 10);
    waits = (unsigned)strtoul(argv[3], nullptr, 10);
    if (argc == 5)
        os_event_set(e);
    if (argc == 5 && !strcmp(argv[4], "taken"))
        os_event_wait(e, 0);
    short_waits = count_short(e, nanos, waits);
    printf("%u of %u waits took less than %llu ns\n", short_waits, waits,
        (unsigned long long)nanos);
    os_event_free(e);
    return (0);
}

int main(int argc, char **argv)
{
    return (argc > 1 ? run(argc, argv) : rows_run(run));
}

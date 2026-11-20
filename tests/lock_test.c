/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "os.h"
#include "rows.h"

static int lock_twice(const char *path, bool unlock)
{
    os_lock_t *first = nullptr;
    os_lock_t *second = nullptr;
    int took = os_lock(path, &first);
    int again = 0;

    if (unlock) {
        os_unlock(first);
        first = nullptr;
    }
    again = os_lock(path, &second);
    printf("lock: %d, then %d\n", took, again);
    os_unlock(second);
    os_unlock(first);
    return (0);
}

static int hold(const char *path)
{
    os_lock_t *lock = nullptr;
    char line[16] = "";

    printf("hold: %d\n", os_lock(path, &lock));
    fflush(stdout);
    if (!fgets(line, sizeof(line), stdin))
        printf("hold: no end\n");
    os_unlock(lock);
    return (0);
}

static int run(int argc, char **argv)
{
    if (argc == 4 && !strcmp(argv[1], "lock"))
        return (lock_twice(argv[2], !strcmp(argv[3], "unlock")));
    if (argc == 3 && !strcmp(argv[1], "hold"))
        return (hold(argv[2]));
    return (printf("usage: lock lock|hold <path> [unlock|keep]\n"), 2);
}

int main(int argc, char **argv)
{
    return (argc > 1 ? run(argc, argv) : rows_run(run));
}

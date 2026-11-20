/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <process.h>
#else
    #include <fcntl.h>
    #include <spawn.h>
#endif
#include "os.h"
#include "rows.h"

constexpr int RETRIES = 1000;
constexpr uint64_t RETRY_NANOS = 10'000'000;
constexpr int FILES_SCANNED = 64;

#ifdef _WIN32
constexpr unsigned char FD_OPEN = 0x01;
#else
extern char **environ;
#endif

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

static int write_locked(const char *path)
{
    os_lock_t *lock = nullptr;
    int took = os_lock(path, &lock);
    FILE *other = fopen(path, "r+b");
    bool made = other && fputc('x', other) != EOF && !fflush(other);

    if (other)
        fclose(other);
    printf("write: %d, %s\n", took, made ? "made" : "refused");
    os_unlock(lock);
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

#ifdef _WIN32
static int start_child(const char *program)
{
    const char *args[] = {"lock", "child", nullptr};

    return (_spawnv(_P_NOWAIT, program, args) == -1);
}

static int count_files(void)
{
    STARTUPINFOA info = {0};
    int count = 0;
    int files = 0;

    GetStartupInfoA(&info);
    if (info.lpReserved2 && info.cbReserved2 >= sizeof(count))
        memcpy(&count, info.lpReserved2, sizeof(count));
    for (int fd = 3; fd < count && fd < FILES_SCANNED
        && sizeof(count) + fd < info.cbReserved2; fd++)
        files += info.lpReserved2[sizeof(count) + fd] & FD_OPEN;
    return (files);
}
#else
static int start_child(const char *program)
{
    char *args[] = {"lock", "child", nullptr};
    pid_t child = 0;

    return (posix_spawn(&child, program, nullptr, nullptr, args, environ));
}

static int count_files(void)
{
    int files = 0;

    for (int fd = 3; fd < FILES_SCANNED; fd++)
        files += fcntl(fd, F_GETFD) != -1;
    return (files);
}
#endif

static int leave(const char *program, const char *path, const char *child)
{
    os_lock_t *lock = nullptr;
    int took = os_lock(path, &lock);
    FILE *shared = strcmp(child, "share") ? nullptr : fopen(path, "rb");

    printf("leave: %d\n", took);
    if (strcmp(child, "none") && start_child(program))
        printf("leave: no child\n");
    if (shared)
        fclose(shared);
    fflush(stdout);
    _Exit(0);
}

static int wait_input(void)
{
    char line[16] = "";
    bool ended = fgets(line, sizeof(line), stdin);

    printf("child: %s, %d files\n", ended ? "end" : "no end", count_files());
    return (0);
}

static int lock_after(const char *path)
{
    os_lock_t *lock = nullptr;
    os_event_t *pause = os_event_create();
    int took = pause ? 1 : -1;

    for (int i = 0; took == 1 && i < RETRIES; i++) {
        if (i)
            os_event_wait(pause, RETRY_NANOS);
        took = os_lock(path, &lock);
    }
    printf("retry: %d\n", took);
    os_unlock(lock);
    os_event_free(pause);
    return (0);
}

static int run(int argc, char **argv)
{
    if (argc == 4 && !strcmp(argv[1], "lock"))
        return (lock_twice(argv[2], !strcmp(argv[3], "unlock")));
    if (argc == 3 && !strcmp(argv[1], "hold"))
        return (hold(argv[2]));
    if (argc == 3 && !strcmp(argv[1], "write"))
        return (write_locked(argv[2]));
    if (argc == 4 && !strcmp(argv[1], "leave"))
        return (leave(argv[0], argv[2], argv[3]));
    if (argc == 2 && !strcmp(argv[1], "child"))
        return (wait_input());
    if (argc == 3 && !strcmp(argv[1], "retry"))
        return (lock_after(argv[2]));
    return (printf("usage: lock lock|hold|write|leave|child|retry "
        "<path> [unlock|keep|none|start|share]\n"), 2);
}

int main(int argc, char **argv)
{
    return (argc > 1 ? run(argc, argv) : rows_run(run));
}

/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#endif

#include "os.h"

constexpr size_t STACK = 1 << 20;

static void set_flag(void *arg)
{
    *(int *)arg = 7;
}

static void set_later(void *arg)
{
    os_event_t *pause = os_event_create();

    if (pause)
        os_event_wait(pause, OS_SECOND / 20);
    os_event_free(pause);
    os_event_set(arg);
}

static int wait_for_thread(void)
{
    os_event_t *e = os_event_create();
    os_thread_t *t;

    if (!e)
        return (printf("FAIL: event to wait for\n"), 1);
    t = os_thread_start(set_later, e, STACK);
    if (!t)
        return (printf("FAIL: thread that sets\n"), 1);
    os_event_wait(e, OS_FOREVER);
    os_thread_join(t);
    os_event_free(e);
    return (0);
}

#ifdef _WIN32
static void guarantee_more(void *arg)
{
    size_t *fall = arg;
    size_t before = os_thread_stack_left();
    ULONG more = 65536;

    SetThreadStackGuarantee(&more);
    *fall = before - os_thread_stack_left();
}

static int check_guarantee(void)
{
    size_t fall = 0;
    os_thread_t *t = os_thread_start(guarantee_more, &fall, STACK);

    if (!t)
        return (printf("FAIL: thread of the guarantee\n"), 1);
    os_thread_join(t);
    if (fall != 65536 - 8192)
        return (printf("FAIL: count the stack guarantee\n"), 1);
    return (0);
}
#endif

static int list_folder(os_folder_t *f)
{
    bool is_folder = false;
    bool small = false;
    bool sub = false;
    int count = 0;
    const char *name;

    while ((name = os_folder_read_entry(f, &is_folder))) {
        small |= !strcmp(name, "small.txt") && !is_folder;
        sub |= !strcmp(name, "sub") && is_folder;
        count++;
    }
    if (count != 2 || !small || !sub)
        return (printf("FAIL: list a folder\n"), 1);
    return (0);
}

static int size_folder(os_folder_t *f, const char *longname)
{
    uint64_t size = 0;

    if (os_folder_entry_size(f, "small.txt", &size) || size != 5)
        return (printf("FAIL: give the size of a file of a folder\n"), 1);
    if (os_folder_entry_size(f, "sub", &size) != -1)
        return (printf("FAIL: give the size of a folder\n"), 1);
    if (os_folder_entry_size(f, "missing.txt", &size) != -1)
        return (printf("FAIL: give the size of a file that is not there\n"), 1);
    if (os_folder_entry_size(f, longname, &size) != -1 || size != 5)
        return (printf("FAIL: give the size of a name that is too long\n"), 1);
    return (0);
}

static int read_folder(os_folder_t *f, const char *longname)
{
    size_t len = 0;
    const char *bytes = os_folder_read_file(f, "small.txt", &len);

    if (!bytes || len != 5 || memcmp(bytes, "small", 6))
        return (printf("FAIL: read a file of a folder\n"), 1);
    if (os_folder_read_file(f, "sub", &len))
        return (printf("FAIL: read a folder as a file\n"), 1);
    if (os_folder_read_file(f, "missing.txt", &len))
        return (printf("FAIL: read a file that is not there\n"), 1);
    if (os_folder_read_file(f, longname, &len))
        return (printf("FAIL: read a name that is too long\n"), 1);
    return (size_folder(f, longname));
}

static int refuse_folders(const char *dir)
{
    char path[8192];
    char name[201] = {};
    size_t at = (size_t)snprintf(path, sizeof(path), "%s/missing", dir);

    if (os_folder_open("") || os_folder_open(path))
        return (printf("FAIL: open a folder that is not there\n"), 1);
    memset(name, 'm', sizeof(name) - 1);
    while (at < 5000)
        at += (size_t)snprintf(path + at, sizeof(path) - at, "/%s", name);
    if (os_folder_open(path))
        return (printf("FAIL: open a long path that is not there\n"), 1);
    if (os_folder_open("//127.0.0.1/no_share"))
        return (printf("FAIL: open a share that is not there\n"), 1);
    return (0);
}

#ifdef _WIN32
static int open_forms(const char *folder)
{
    char *whole = os_path_whole(folder);
    char path[4096];
    os_folder_t *f;
    int opened = 0;

    for (int i = 0; whole && i < 2; i++) {
        snprintf(path, sizeof(path), "%s%s", i ? "\\\\.\\" : "\\\\?\\", whole);
        f = os_folder_open(path);
        opened += f != nullptr;
        os_folder_close(f);
    }
    free(whole);
    if (opened != 2)
        return (printf("FAIL: open a folder in a form of a device\n"), 1);
    return (0);
}
#endif

static int check_folders(const char *dir, const char *longname)
{
    char path[4096];
    os_folder_t *f;
    FILE *small;
    int failed;

    snprintf(path, sizeof(path), "%s/folder", dir);
    if (os_folder_make(path))
        return (printf("FAIL: make the folder to list\n"), 1);
    snprintf(path, sizeof(path), "%s/folder/sub", dir);
    if (os_folder_make(path))
        return (printf("FAIL: make the folder to list\n"), 1);
    snprintf(path, sizeof(path), "%s/folder/small.txt", dir);
    small = fopen(path, "wb");
    if (!small || fputs("small", small) < 0 || fclose(small))
        return (printf("FAIL: write the file to read\n"), 1);
    snprintf(path, sizeof(path), "%s/folder", dir);
    f = os_folder_open(path);
    if (!f)
        return (printf("FAIL: open a folder\n"), 1);
    failed = list_folder(f) || read_folder(f, longname);
    os_folder_close(f);
    os_folder_close(nullptr);
#ifdef _WIN32
    failed = failed || open_forms(path);
#endif
    return (failed || refuse_folders(dir));
}

int main(int argc, char **argv)
{
    const char *dir;
    char path[4096];
    char longname[300];
    void *base;
    os_event_t *e;
    os_thread_t *t;
    os_mutex_t *m;
    char *whole;
    void *lib;
    void *sym;
    int (*answer)(void);
    int flag = 0;

    if (argc < 2)
        return (printf("FAIL: no workdir\n"), 1);
    dir = argv[1];

    base = os_reserve(8192, 4096);
    if (!base)
        return (printf("FAIL: reserve\n"), 1);
    if (os_commit(base, 8192))
        return (printf("FAIL: commit\n"), 1);
    os_release(base, 8192);
    if (os_reserve((size_t)-1, 4096))
        return (printf("FAIL: reserve past the address space\n"), 1);
    if (os_reserve(4096, (size_t)-1))
        return (printf("FAIL: commit past the reservation\n"), 1);
    base = os_reserve(8192, 4096);
    if (!base)
        return (printf("FAIL: reserve again\n"), 1);
    if (!os_commit(base, (size_t)-1))
        return (printf("FAIL: commit a bad size\n"), 1);
    os_release(base, 8192);

    e = os_event_create();
    if (!e)
        return (printf("FAIL: event\n"), 1);
    os_event_wait(e, OS_SECOND / 1000);
    os_event_set(e);
    os_event_wait(e, OS_SECOND / 1000);
    os_event_set(e);
    os_event_wait(e, OS_FOREVER);
    os_event_free(e);
    os_event_free(nullptr);
    if (wait_for_thread())
        return (1);

    t = os_thread_start(set_flag, &flag, 1);
    if (!t)
        return (printf("FAIL: thread\n"), 1);
    os_thread_join(t);
    if (flag != 7)
        return (printf("FAIL: thread work\n"), 1);
    if (os_thread_start(set_flag, &flag, SIZE_MAX))
        return (printf("FAIL: a stack larger than the address space\n"), 1);
#ifdef _WIN32
    if (check_guarantee())
        return (1);
#endif
    if (!os_get_thread_id())
        return (printf("FAIL: thread id\n"), 1);
    (void)os_get_now();

    m = os_mutex_create();
    if (!m)
        return (printf("FAIL: mutex\n"), 1);
    os_mutex_lock(m);
    os_mutex_unlock(m);
    os_mutex_free(m);
    os_mutex_free(nullptr);

    snprintf(path, sizeof(path), "%s/lock.tmp", dir);
    if (os_lock(path))
        return (printf("FAIL: lock\n"), 1);
    if (os_lock(path) != 1)
        return (printf("FAIL: relock\n"), 1);
    if (os_lock("nowhere_dir/lock.tmp") != -1)
        return (printf("FAIL: lock a bad path\n"), 1);

    whole = os_path_whole(".");
    if (!whole)
        return (printf("FAIL: path\n"), 1);
    free(whole);

    snprintf(path, sizeof(path), "%s/new_folder", dir);
    if (os_folder_make(path))
        return (printf("FAIL: make a folder\n"), 1);
    if (os_folder_make(path))
        return (printf("FAIL: make an existing folder\n"), 1);
    snprintf(path, sizeof(path), "%s/lock.tmp", dir);
    if (os_folder_make(path) != -1)
        return (printf("FAIL: make a folder over a file\n"), 1);
    if (os_folder_set("."))
        return (printf("FAIL: set the folder\n"), 1);

    memset(longname, 'a', sizeof(longname) - 1);
    longname[sizeof(longname) - 1] = '\0';
    if (check_folders(dir, longname))
        return (1);
    if (os_lib_open(longname))
        return (printf("FAIL: open a long name\n"), 1);
    if (os_lib_open("no_such_library"))
        return (printf("FAIL: open a bare name\n"), 1);
    if (os_lib_open("no/such/library"))
        return (printf("FAIL: open a path with a slash\n"), 1);
    if (os_lib_open("no\\such\\library"))
        return (printf("FAIL: open a path with a backslash\n"), 1);
    lib = os_lib_open(FIXTURE_PATH);
    if (!lib)
        return (printf("FAIL: open the fixture\n"), 1);
    sym = os_lib_get_symbol(lib, "fixture_answer");
    if (!sym)
        return (printf("FAIL: find the symbol\n"), 1);
    memcpy(&answer, &sym, sizeof(answer));
    if (answer() != 42)
        return (printf("FAIL: call the symbol\n"), 1);
    os_lib_close(lib);
    if (os_lib_pin(FIXTURE_PATH) != -1)
        return (printf("FAIL: pin a closed library\n"), 1);
    if (os_lib_pin(longname) != -1 || os_lib_pin("no_such_library") != -1)
        return (printf("FAIL: pin a library that is not open\n"), 1);
    lib = os_lib_open(FIXTURE_PATH);
    if (!lib || os_lib_pin(FIXTURE_PATH))
        return (printf("FAIL: pin the fixture\n"), 1);
    os_lib_close(lib);
    if (os_lib_pin(FIXTURE_PATH))
        return (printf("FAIL: keep a pinned library after its close\n"), 1);
    lib = os_lib_open(FIXTURE_TWO_PATH);
    if (!lib || os_lib_pin(FIXTURE_TWO_PATH))
        return (printf("FAIL: pin a second library\n"), 1);
    os_lib_close(lib);

    printf("os ok\n");
    return (0);
}

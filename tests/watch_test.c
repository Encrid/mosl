/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifdef _WIN32
    #include <direct.h>
#else
    #include <sys/stat.h>
    #include <unistd.h>
#endif
#include "os.h"
#include "rows.h"

constexpr int WAITS = 10000;
constexpr int BURST = 100;
constexpr int FLOOD = 10000;
constexpr int TAKES = 1000;
constexpr int TRIES = 100;
constexpr int WANTED = 4;
constexpr int SPOTS = 8;

typedef struct waiting {
    os_watch_t *w;
    const char *end;
    const char *wanted[WANTED];
    bool found[WANTED];
    bool burst[BURST];
    int status;
} waiting_t;

static const char *base;
static char spots[SPOTS][1024];
static int spot;
static os_event_t *pause_event;

static const char *at(const char *name)
{
    char *out = spots[spot];

    spot = (spot + 1) % SPOTS;
    snprintf(out, sizeof(spots[0]), "%s/%s", base, name);
    return (out);
}

static void make_folder(const char *name)
{
#ifdef _WIN32
    _mkdir(at(name));
#else
    mkdir(at(name), 0755);
#endif
}

static void drop_folder(const char *name)
{
#ifdef _WIN32
    _rmdir(at(name));
#else
    rmdir(at(name));
#endif
}

static void put_file(const char *name, const char *how)
{
    FILE *f = fopen(at(name), how);

    if (!f)
        return;
    fputs("x", f);
    fclose(f);
}

static void pause_for(uint64_t nanos)
{
    if (!pause_event)
        pause_event = os_event_create();
    if (pause_event)
        os_event_wait(pause_event, nanos);
}

static void note(waiting_t *t, const char *name)
{
    int index = -1;

    for (int i = 0; i < WANTED; i++)
        if (t->wanted[i] && !strcmp(name, t->wanted[i]))
            t->found[i] = true;
    if (sscanf(name, "burst_%d.txt", &index) == 1 && index >= 0
        && index < BURST)
        t->burst[index] = true;
}

static void take_until(waiting_t *t, bool at_lost)
{
    const char *name = nullptr;
    int got = 0;

    for (int i = 0; i < WAITS; i++) {
        got = os_watch_take(t->w, &name);
        while (got == 1 && !(t->end && !strcmp(name, t->end))) {
            note(t, name);
            got = os_watch_take(t->w, &name);
        }
        if (got == 1 || got == -1 || (got == 2 && at_lost)) {
            t->status = got;
            return;
        }
        pause_for(OS_SECOND / 1000);
    }
    t->status = 0;
}

static bool start(waiting_t *t, const char *folder, uint32_t depth,
    const char *mode)
{
    t->w = os_watch_start(at(folder), depth);
    if (!t->w)
        printf("%s: not watched\n", mode);
    return (t->w != nullptr);
}

static void settle(waiting_t *t)
{
    const char *end = t->end;

    put_file("watched/ready", "w");
    t->end = "ready";
    take_until(t, false);
    t->end = end;
    memset(t->found, 0, sizeof(t->found));
}

static const char *said(bool yes, const char *what)
{
    return (yes ? what : "nothing");
}

static void run_new(bool beside)
{
    waiting_t t = { .end = "marker", .wanted = { "a.txt" } };

    if (!start(&t, "watched", 1, "new"))
        return;
    put_file(beside ? "beside/a.txt" : "watched/a.txt", "w");
    put_file("watched/marker", "w");
    take_until(&t, false);
    printf("new: %s\n", said(t.found[0], "a.txt"));
    os_watch_stop(t.w);
}

static void run_changed(bool other, bool removed)
{
    const char *file = other ? "watched/b.txt" : "watched/a.txt";
    waiting_t t = { .end = "marker", .wanted = { "a.txt" } };

    put_file(file, "w");
    if (!start(&t, "watched", 1, removed ? "removed" : "changed"))
        return;
    settle(&t);
    if (removed)
        remove(at(file));
    else
        put_file(file, "a");
    put_file("watched/marker", "w");
    take_until(&t, false);
    printf("%s: %s\n", removed ? "removed" : "changed",
        said(t.found[0], "a.txt"));
    os_watch_stop(t.w);
}

static void run_depth(bool flat)
{
    waiting_t two = { .end = "marker", .wanted = { "sub/b.txt",
        "sub/deeper/c.txt" } };
    waiting_t one = { .end = "marker", .wanted = { "sub/b.txt" } };

    make_folder("watched/sub");
    make_folder("watched/sub/deeper");
    if (!start(&two, "watched", 2, "depth") || !start(&one, "watched", 1,
        "depth"))
        return;
    settle(&two);
    settle(&one);
    put_file(flat ? "watched/b.txt" : "watched/sub/b.txt", "w");
    put_file("watched/sub/deeper/c.txt", "w");
    put_file("watched/marker", "w");
    take_until(&two, false);
    take_until(&one, false);
    printf("depth: %s at 2, %s at 1, %s at 2\n", said(two.found[0],
        "sub/b.txt"), said(one.found[0], "sub/b.txt"), said(two.found[1],
        "sub/deeper/c.txt"));
    os_watch_stop(two.w);
    os_watch_stop(one.w);
}

static void run_zero(bool one)
{
    os_watch_t *w = os_watch_start(at("watched"), one ? 1 : 0);

    printf("zero: %s\n", w ? "watched" : "refused");
    os_watch_stop(w);
}

static void run_quiet(bool busy)
{
    waiting_t t = {};
    const char *name = nullptr;
    uint64_t begin = 0;
    int got = 0;
    int zero = 0;

    if (!start(&t, "watched", 1, "quiet"))
        return;
    if (busy)
        put_file("watched/a.txt", "w");
    if (busy)
        pause_for(OS_SECOND / 5);
    begin = os_get_now();
    for (int i = 0; i < TAKES; i++) {
        got = os_watch_take(t.w, &name);
        zero += got == 0;
    }
    printf("quiet: %d of %d takes gave 0, %s\n", zero, TAKES,
        os_get_now() - begin < OS_SECOND ? "at once" : "slowly");
    os_watch_stop(t.w);
}

static void run_burst(bool beside)
{
    waiting_t t = { .end = "marker" };
    char name[64];
    int seen = 0;

    if (!start(&t, "watched", 1, "burst"))
        return;
    for (int i = 0; i < BURST; i++) {
        snprintf(name, sizeof(name), "%s/burst_%03d.txt",
            beside ? "beside" : "watched", i);
        put_file(name, "w");
    }
    put_file("watched/marker", "w");
    take_until(&t, false);
    for (int i = 0; i < BURST; i++)
        seen += t.burst[i];
    printf("burst: %d of %d\n", seen, BURST);
    os_watch_stop(t.w);
}

static void run_overflow(bool few)
{
    waiting_t t = { .end = "marker", .wanted = { "after.txt" } };
    char name[64];
    bool lost = false;

    if (!start(&t, "watched", 1, "overflow"))
        return;
    for (int i = 0; i < (few ? BURST : FLOOD); i++) {
        snprintf(name, sizeof(name), "watched/f%05d.txt", i);
        put_file(name, "w");
    }
    put_file("watched/marker", "w");
    take_until(&t, true);
    lost = t.status == 2;
    put_file("watched/after.txt", "w");
    put_file("watched/marker2", "w");
    t.end = "marker2";
    take_until(&t, false);
    printf("overflow: %s, then %s\n", lost ? "2" : "no 2",
        said(t.found[0], "after.txt"));
    os_watch_stop(t.w);
}

static void run_gone(bool beside)
{
    waiting_t t = { .end = "marker" };
    int again = 0;
    const char *name = nullptr;

    if (!start(&t, "watched", 1, "gone"))
        return;
    drop_folder(beside ? "beside" : "watched");
    put_file("watched/marker", "w");
    take_until(&t, false);
    again = os_watch_take(t.w, &name);
    if (t.status == -1)
        printf("gone: -1, then %d\n", again);
    else
        printf("gone: still watching\n");
    os_watch_stop(t.w);
}

static void run_moved(bool beside)
{
    waiting_t same = { .end = "marker" };
    waiting_t longer = { .end = "marker" };

    make_folder("other");
    if (!start(&same, "watched", 1, "moved") || !start(&longer, "other", 1,
        "moved"))
        return;
    rename(at(beside ? "beside" : "watched"), at("wetched"));
    rename(at(beside ? "wetched" : "other"), at("other_moved"));
    put_file(beside ? "watched/marker" : "wetched/marker", "w");
    put_file(beside ? "other/marker" : "other_moved/marker", "w");
    take_until(&same, false);
    take_until(&longer, false);
    if (same.status == -1 && longer.status == -1)
        printf("moved: -1 and -1\n");
    else
        printf("moved: still watching\n");
    os_watch_stop(same.w);
    os_watch_stop(longer.w);
}

static void run_beside(bool moved)
{
    waiting_t t = { .end = "marker" };

    if (!start(&t, "watched", 1, "beside"))
        return;
    if (moved)
        rename(at("watched"), at("wetched"));
    else
        make_folder("beside2");
    put_file(moved ? "wetched/marker" : "watched/marker", "w");
    take_until(&t, false);
    printf("beside: %s\n", t.status == 1 ? "marker, still watching" : "-1");
    os_watch_stop(t.w);
}

static void run_folders(bool flat)
{
    waiting_t t = { .end = "new", .wanted = { "new/x.txt", "new/inner",
        "sub" } };

    make_folder("watched/sub");
    if (!start(&t, "watched", flat ? 1 : 2, "folders"))
        return;
    settle(&t);
    make_folder("watched/new");
    take_until(&t, false);
    put_file("watched/new/x.txt", "w");
    make_folder("watched/new/inner");
    drop_folder("watched/sub");
    put_file("watched/marker", "w");
    t.end = "marker";
    take_until(&t, false);
    printf("folders: new, %s, %s and %s\n", said(t.found[0], "new/x.txt"),
        said(t.found[1], "new/inner"), said(t.found[2], "sub"));
    os_watch_stop(t.w);
}

static void run_renamed(bool still)
{
    waiting_t t = { .end = "moved", .wanted = { "sub", "moved/inner/x.txt" } };

    make_folder("watched/sub");
    make_folder("watched/sub/inner");
    make_folder("watched/subway");
    if (!start(&t, "watched", 3, "renamed"))
        return;
    settle(&t);
    if (!still)
        rename(at("watched/sub"), at("watched/moved"));
    else
        make_folder("watched/moved");
    take_until(&t, false);
    put_file(still ? "watched/sub/inner/x.txt" : "watched/moved/inner/x.txt",
        "w");
    put_file("watched/marker", "w");
    t.end = "marker";
    take_until(&t, false);
    printf("renamed: %s, moved and %s\n", said(t.found[0], "sub"),
        said(t.found[1], "moved/inner/x.txt"));
    os_watch_stop(t.w);
}

static void run_root(bool missing)
{
    char root[4] = "/";
    os_watch_t *w = nullptr;
    const char *name = nullptr;
    int got = -1;

    if (base[0] && base[1] == ':')
        snprintf(root, sizeof(root), "%c:/", base[0]);
    w = os_watch_start(missing ? at("missing/") : root, 1);
    if (w)
        got = os_watch_take(w, &name);
    printf("root: %s\n", got >= 0 ? "watched" : "refused");
    os_watch_stop(w);
}

static void run_missing(bool made)
{
    os_watch_t *folder = nullptr;
    os_watch_t *file = nullptr;

    if (made)
        make_folder("nothing");
    put_file("watched/file.txt", "w");
    folder = os_watch_start(at("nothing"), 1);
    file = os_watch_start(at("watched/file.txt"), 1);
    printf("missing: %s, file: %s\n", folder ? "watched" : "refused",
        file ? "watched" : "refused");
    os_watch_stop(folder);
    os_watch_stop(file);
}

static void run_refuse(void)
{
    char name[64];
    os_watch_t *w = nullptr;
    int refused = 0;

    for (int i = 0; i < 20; i++) {
        snprintf(name, sizeof(name), "watched/folder_%02d", i);
        make_folder(name);
    }
    put_file("watched/file.txt", "w");
    for (int i = 0; i < TRIES && !w; i++) {
        w = os_watch_start(at("watched"), 2);
        refused += !w;
    }
    if (w)
        printf("refuse: %d refused, then made\n", refused);
    os_watch_stop(w);
}

static bool run_mode(const char *mode, bool other)
{
    if (!strcmp(mode, "new"))
        run_new(other);
    else if (!strcmp(mode, "changed") || !strcmp(mode, "removed"))
        run_changed(other, !strcmp(mode, "removed"));
    else if (!strcmp(mode, "depth"))
        run_depth(other);
    else if (!strcmp(mode, "zero"))
        run_zero(other);
    else if (!strcmp(mode, "quiet"))
        run_quiet(other);
    else if (!strcmp(mode, "burst"))
        run_burst(other);
    else if (!strcmp(mode, "overflow"))
        run_overflow(other);
    else
        return (false);
    return (true);
}

static bool run_place(const char *mode, bool other)
{
    if (!strcmp(mode, "gone"))
        run_gone(other);
    else if (!strcmp(mode, "moved"))
        run_moved(other);
    else if (!strcmp(mode, "beside"))
        run_beside(other);
    else if (!strcmp(mode, "folders"))
        run_folders(other);
    else if (!strcmp(mode, "renamed"))
        run_renamed(other);
    else if (!strcmp(mode, "root"))
        run_root(other);
    else if (!strcmp(mode, "missing"))
        run_missing(other);
    else if (!strcmp(mode, "refuse"))
        run_refuse();
    else
        return (false);
    return (true);
}

static int run(int argc, char **argv)
{
    static char run[1024];
    char name[64];
    bool other = argc == 4;

    if (argc < 3)
        return (printf("usage: watch <folder> <mode> [other]\n"), 2);
    snprintf(name, sizeof(name), "run_%llu", (unsigned long long)os_get_now());
    base = argv[1];
    make_folder(name);
    snprintf(run, sizeof(run), "%s/%s", argv[1], name);
    base = run;
    make_folder("watched");
    make_folder("beside");
    if (!run_mode(argv[2], other) && !run_place(argv[2], other))
        printf("watch: no mode %s\n", argv[2]);
    os_event_free(pause_event);
    pause_event = nullptr;
    return (0);
}

int main(int argc, char **argv)
{
    return (argc > 1 ? run(argc, argv) : rows_run(run));
}

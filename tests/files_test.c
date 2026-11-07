/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#else
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "os.h"

constexpr int LEVELS = 22;
constexpr size_t LEVEL = 200;
constexpr char TOLD[] = "as The files tells";

static const char *folder;

static void setup(int status)
{
    if (status)
        printf("setup: a call failed\n");
}

static char *join(const char *a, const char *b)
{
    size_t size = strlen(a) + strlen(b) + 2;
    char *path = malloc(size);

    if (path)
        snprintf(path, size, "%s/%s", a, b);
    return (path);
}

static char *in(const char *name)
{
    static char *paths[8];
    static int next;

    free(paths[next]);
    paths[next] = join(folder, name);
    name = paths[next];
    next = (next + 1) % 8;
    return ((char *)name);
}

static const char *show(const char *at, const char *name)
{
    static char text[64];
    os_folder_t *f = os_folder_open(at);
    size_t len = 0;
    char *bytes = f ? os_folder_read_file(f, name, &len) : nullptr;

    if (bytes)
        snprintf(text, sizeof(text), "%.*s", (int)len, bytes);
    else
        snprintf(text, sizeof(text), "none");
    os_folder_close(f);
    return (text);
}

static void make_read_only(const char *path)
{
#ifdef _WIN32
    SetFileAttributesA(path, FILE_ATTRIBUTE_READONLY);
#else
    struct stat st = {};

    if (!stat(path, &st))
        chmod(path, S_ISDIR(st.st_mode) ? 0555 : 0444);
#endif
}

static void make_writable(const char *path)
{
#ifdef _WIN32
    SetFileAttributesA(path, FILE_ATTRIBUTE_NORMAL);
#else
    struct stat st = {};

    if (!stat(path, &st))
        chmod(path, S_ISDIR(st.st_mode) ? 0755 : 0644);
#endif
}

static int make(char **argv)
{
    char *path = in(argv[3]);
    int made = os_folder_make(path);

    return (printf("make: %d %d\n", made, os_path_entry(path)), 0);
}

static char *deep_path(void)
{
    size_t size = strlen(folder) + LEVELS * (LEVEL + 1) + 1;
    char *path = malloc(size);
    size_t at = 0;

    if (!path)
        return (nullptr);
    at = (size_t)snprintf(path, size, "%s", folder);
    for (int i = 0; i < LEVELS; i++) {
        path[at++] = '/';
        memset(path + at, 'd', LEVEL);
        at += LEVEL;
    }
    path[at] = '\0';
    return (path);
}

static int deep(char **argv)
{
    char *path = deep_path();
    char *first = path ? join(path, "") : nullptr;
    char *file = path ? join(path, "f") : nullptr;
    char *moved = path ? join(path, "g") : nullptr;
    char *link = path ? join(path, "l") : nullptr;
    int made = 1;
    int wrote = 0;

    if (!first || !file || !moved || !link)
        return (printf("deep: no memory\n"), 1);
    first[strlen(folder) + 1 + LEVEL] = '\0';
    if (!strcmp(argv[3], "made"))
        made = os_folder_make(path);
    wrote = os_file_write(file, "deep file", 9, false);
    printf("deep: make %d, write %d, read %s, entry %d, ", made, wrote,
        show(path, "f"), os_path_entry(file));
    printf("move %d, entry %d, ", os_path_move(file, moved),
        os_path_entry(moved));
    printf("link %d, entry %d, ", os_link_make(link, "."),
        os_path_entry(link));
    printf("remove %d, entry %d\n", os_path_remove(first),
        os_path_entry(first));
    free(path);
    free(first);
    free(file);
    free(moved);
    free(link);
    return (0);
}

static int locked(char **)
{
    int wrote = 0;

    make_read_only(in("f"));
    wrote = os_file_write(in("f"), "f", 1, false);
    make_writable(in("f"));
    return (printf("locked: %d\n", wrote), 0);
}

static int replace(char **argv)
{
    int wrote = os_file_write(in("f"), argv[3], strlen(argv[3]), false);
    const char *text = show(folder, "f");

    return (printf("replace: %d, %zu bytes: %s\n", wrote, strlen(text), text),
        0);
}

static int mode(char **argv)
{
    char *path = in("m");
    bool executable = argv[4][0] == 'x';
    int wrote = 0;
#ifdef _WIN32
    FILE *before = strcmp(argv[3], "none") ? fopen(path, "wb") : nullptr;

    if (before)
        fclose(before);
    wrote = os_file_write(path, "m", 1, executable);
    if (!wrote && os_path_entry(path) == 1)
        return (printf("mode: %s\n", TOLD), 0);
    return (printf("mode: %d %d\n", wrote, os_path_entry(path)), 0);
#else
    long want = strtol(argv[5], nullptr, 8);
    struct stat st = {};
    int fd = -1;

    umask(022);
    if (strcmp(argv[3], "none"))
        fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd >= 0) {
        fchmod(fd, (mode_t)strtol(argv[3], nullptr, 8));
        close(fd);
    }
    wrote = os_file_write(path, "m", 1, executable);
    if (!wrote && !stat(path, &st) && (st.st_mode & 07777) == want)
        return (printf("mode: %s\n", TOLD), 0);
    return (printf("mode: %d %04o\n", wrote, st.st_mode & 07777), 0);
#endif
}

static int entry(char **)
{
    char *file = in("f");
    char *made = in("d");

    if (os_file_write(file, "f", 1, false) || os_folder_make(made))
        printf("entry: the file or the folder is not made\n");
    printf("entry: %d %d %d ", os_path_entry(file), os_path_entry(made),
        os_path_entry(in("m")));
    return (printf("%d\n", os_path_entry(in("m/x"))), 0);
}

static int entry_x(char **)
{
    char *path = in("x");
#ifdef _WIN32
    constexpr int WANT = 1;
#else
    constexpr int WANT = 2;
#endif
    int wrote = os_file_write(path, "x", 1, true);
    int found = os_path_entry(path);

    if (!wrote && found == WANT)
        return (printf("entry-x: %s\n", TOLD), 0);
    return (printf("entry-x: %d %d\n", wrote, found), 0);
}

static int entry_link(char **)
{
    char *target = in("t");

    setup(os_folder_make(target));
    setup(os_file_write(in("t/k"), "k", 1, false));
    setup(os_link_make(in("lt"), "t"));
    setup(os_link_make(in("lk"), "t/k"));
    setup(os_link_make(in("ln"), "none"));
    return (printf("entry-link: %d %d %d\n", os_path_entry(in("lt")),
        os_path_entry(in("lk")), os_path_entry(in("ln"))), 0);
}

static int move(char **)
{
    int moved = os_path_move(in("a"), in("b"));

    printf("move: %d, a %d, b holds %s, ", moved, os_path_entry(in("a")),
        show(folder, "b"));
    return (printf("none %d\n", os_path_move(in("none"), in("c"))), 0);
}

static int move_folder(char **)
{
    int moved = 0;

    setup(os_folder_make(in("d")));
    setup(os_file_write(in("d/f"), "f", 1, false));
    moved = os_path_move(in("d"), in("e"));
    return (printf("move-folder: %d, d %d, e/f %d\n", moved,
        os_path_entry(in("d")), os_path_entry(in("e/f"))), 0);
}

static int remove_tree(char **)
{
    int removed = 0;

    setup(os_folder_make(in("d/s")));
    setup(os_folder_make(in("t")));
    setup(os_file_write(in("d/f"), "f", 1, false));
    setup(os_file_write(in("d/s/g"), "g", 1, false));
    setup(os_file_write(in("t/k"), "k", 1, false));
    setup(os_link_make(in("d/l"), "../t"));
    make_read_only(in("d/f"));
    make_read_only(in("d/s"));
    removed = os_path_remove(in("d"));
    printf("remove: %d, d %d, t %d, ", removed, os_path_entry(in("d")),
        os_path_entry(in("t")));
    return (printf("k %d\n", os_path_entry(in("t/k"))), 0);
}

static int link_forms(char **)
{
    char *target = os_path_whole(in("t"));
    int missing = os_folder_set(in("none"));

    if (!target || os_folder_set(folder))
        return (printf("link-forms: no folder\n"), 0);
    printf("link-forms: set %d, ", missing);
    printf("%d %d ", os_link_make("l1", "t"), os_path_entry("l1"));
    printf("%d %d ", os_link_make(in("l2"), target), os_path_entry(in("l2")));
    printf("%d %d ", os_link_make(in("l3"), "/none"), os_path_entry(in("l3")));
    printf("%d %d ", os_link_make(in("l5"), "\\none"),
        os_path_entry(in("l5")));
    printf("%d\n", os_link_make(in("l4"), ""));
    free(target);
    return (0);
}

static int entry_of(char **argv)
{
    return (printf("entry-of: %d\n", os_path_entry(argv[3])), 0);
}

static int remove_none(char **)
{
    printf("remove-none: %d ", os_path_remove(in("none")));
    printf("%d ", os_path_remove(in("none/x")));
    return (printf("%d\n", os_path_remove(in("a/k/x"))), 0);
}
static int move_args(char **argv)
{
    return (printf("move-args: %d\n", os_path_move(argv[3], argv[4])), 0);
}

static int remove_of(char **argv)
{
    return (printf("remove-of: %d\n", os_path_remove(argv[3])), 0);
}

static int locked_folder(char **)
{
    char *ro = in("ro");
    int wrote = 0;
    int made = 0;
    int removed = 0;

    setup(os_folder_make(ro));
    setup(os_folder_make(in("ro/sub")));
    make_read_only(ro);
    wrote = os_file_write(in("ro/f"), "f", 1, false);
    made = os_folder_make(in("ro/new"));
    removed = os_path_remove(in("ro/sub"));
    make_writable(ro);
    printf("locked-write: %d\n", wrote);
    printf("locked-make: %d\n", made);
    printf("locked-remove: %d\n", removed);
    return (0);
}

static int make_file(char **)
{
    char *path = in("mf");

    setup(os_folder_make(path));
    setup(os_path_remove(path));
    setup(os_file_write(path, "f", 1, false));
    return (printf("make-file: %d\n", os_folder_make(path)), 0);
}

static int link_relative(char **argv)
{
    int linked = 0;

    setup(os_folder_make(in("s")));
    setup(os_file_write(in("f"), "f", 1, false));
    linked = os_link_make(in("s/l"), argv[3]);
    return (printf("link-relative: %d, reads %s\n", linked,
        show(in("s"), "l")), 0);
}

#ifndef _WIN32
static int probe_dirfd(const char *at_path, const char *name)
{
    struct stat st = {};
    int fd = open(at_path, O_RDONLY | O_DIRECTORY);
    int made = fd < 0 ? -2 : mkdirat(fd, name, 0777);

    printf("whole: dirfd %d, %d\n", made,
        made ? -1 : (fstatat(fd, name, &st, 0) ? -3 : S_ISDIR(st.st_mode)));
    if (fd >= 0)
        close(fd);
    return (0);
}
#endif

static int whole(char **)
{
    char *path = in("\xc3\xa9");
    int status = os_folder_make(path);
    char *made = status ? nullptr : os_path_whole(path);

    printf("whole: layer %d, entry %d\n", status, os_path_entry(path));
#ifdef _WIN32
    printf("whole: dirfd 0, 0\n");
#else
    probe_dirfd(folder, "\xc3\xa9d");
#endif
    free(made);
    return (0);
}

constexpr int TRIES = 9;
constexpr size_t BIG = 2u << 20;

static bool try_write([[maybe_unused]] int attempt)
{
    return (!os_file_write(in("w"), "w", 1, false));
}

static bool try_write_n(int attempt)
{
    char name[8];

    snprintf(name, sizeof(name), "w%d", attempt);
    return (!os_file_write(in(name), "w", 1, false));
}

static bool try_write_x([[maybe_unused]] int attempt)
{
    return (!os_file_write(in("w"), "w", 1, true));
}

static bool try_entry([[maybe_unused]] int attempt)
{
    return (os_path_entry(in("e")) == 1);
}

static bool try_move([[maybe_unused]] int attempt)
{
    return (!os_path_move(in("a"), in("b")));
}

static bool try_remove([[maybe_unused]] int attempt)
{
    return (!os_path_remove(in("d")));
}

static bool try_link([[maybe_unused]] int attempt)
{
    return (!os_link_make(in("l"), "t"));
}

static bool try_make([[maybe_unused]] int attempt)
{
    return (!os_folder_make(in("a/b")));
}

static bool try_set([[maybe_unused]] int attempt)
{
    return (!os_folder_set(folder));
}

static bool try_whole([[maybe_unused]] int attempt)
{
    char *made = os_path_whole(folder);

    free(made);
    return (made != nullptr);
}

typedef struct {
    const char *name;
    bool (*run)(int attempt);
} try_t;

static const try_t TRIED[] = {
    {"write", try_write}, {"write-n", try_write_n},
    {"write-x", try_write_x}, {"entry", try_entry},
    {"move", try_move}, {"remove", try_remove}, {"link", try_link},
    {"make", try_make}, {"set", try_set}, {"whole", try_whole},
};

static int again(char **argv)
{
    const try_t *chosen = nullptr;
    int refused = 0;

    for (size_t i = 0; i < sizeof(TRIED) / sizeof(TRIED[0]); i++)
        chosen = strcmp(argv[3], TRIED[i].name) ? chosen : &TRIED[i];
    if (!chosen)
        return (printf("usage: files <folder> again <call>\n"), 2);
    make_read_only(in("d/f"));
    make_read_only(in("d/s"));
    while (refused < TRIES && !chosen->run(refused))
        refused++;
    if (refused < TRIES)
        printf("%s: %d refused, then made\n", argv[3], refused);
    else
        printf("%s: always refused\n", argv[3]);
    return (0);
}

static int big(char **)
{
    char *bytes = malloc(BIG);
    int wrote = -1;
    os_folder_t *f = nullptr;
    size_t len = 0;

    if (!bytes)
        return (printf("big: no memory\n"), 1);
    memset(bytes, 'b', BIG);
    wrote = os_file_write(in("big"), bytes, BIG, false);
    f = os_folder_open(folder);
    if (f && os_folder_read_file(f, "big", &len))
        printf("big: %d, %zu bytes\n", wrote, len);
    else
        printf("big: %d, no file\n", wrote);
    os_folder_close(f);
    free(bytes);
    return (0);
}

typedef struct {
    const char *name;
    int count;
    int (*run)(char **argv);
} test_mode_t;

static const test_mode_t MODES[] = {
    {"make", 4, make}, {"deep", 4, deep}, {"replace", 4, replace},
    {"mode", 6, mode}, {"entry", 3, entry}, {"entry-x", 3, entry_x},
    {"entry-link", 3, entry_link}, {"move", 3, move},
    {"move-folder", 3, move_folder}, {"remove", 3, remove_tree},
    {"remove-none", 3, remove_none}, {"link-relative", 4, link_relative},
    {"whole", 3, whole}, {"again", 4, again}, {"big", 3, big},
    {"link-forms", 3, link_forms}, {"entry-of", 4, entry_of},
    {"locked", 3, locked}, {"move-args", 5, move_args},
    {"remove-of", 4, remove_of}, {"locked-folder", 3, locked_folder},
    {"make-file", 3, make_file},
};

int main(int argc, char **argv)
{
    if (argc < 3)
        return (printf("usage: files <folder> <mode> [arguments]\n"), 2);
    folder = argv[1];
    for (size_t i = 0; i < sizeof(MODES) / sizeof(MODES[0]); i++) {
        if (!strcmp(argv[2], MODES[i].name) && argc == MODES[i].count)
            return (MODES[i].run(argv));
    }
    return (printf("usage: files <folder> <mode> [arguments]\n"), 2);
}

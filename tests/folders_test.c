/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <sddl.h>
#else
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif
#include "os.h"
#include "rows.h"

constexpr int LEVELS = 22;
constexpr int TRIES = 4;
constexpr char TEXT[] = "deep file";

static char level[201];
static char file[256];

#ifdef _WIN32
static wchar_t chain[16384];
static size_t ends[LEVELS + 1];

static void add_name(const char *name)
{
    size_t at = wcslen(chain);

    chain[at] = L'\\';
    MultiByteToWideChar(CP_UTF8, 0, name, -1, chain + at + 1, 256);
}

static void make_chain(const char *base)
{
    wchar_t wide[4096];
    HANDLE made;
    DWORD put;

    MultiByteToWideChar(CP_UTF8, 0, base, -1, wide, 4096);
    wcscpy(chain, L"\\\\?\\");
    GetFullPathNameW(wide, 4096, chain + 4, nullptr);
    for (int i = 1; i <= LEVELS; i++) {
        add_name(level);
        CreateDirectoryW(chain, nullptr);
        ends[i] = wcslen(chain);
    }
    add_name(file);
    made = CreateFileW(chain, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, nullptr);
    WriteFile(made, TEXT, sizeof(TEXT) - 1, &put, nullptr);
    CloseHandle(made);
}

static void remove_chain([[maybe_unused]] const char *base)
{
    DeleteFileW(chain);
    for (int i = LEVELS; i > 0; i--) {
        chain[ends[i]] = L'\0';
        RemoveDirectoryW(chain);
    }
}

static void lock([[maybe_unused]] const char *base, bool last, bool on)
{
    PSECURITY_DESCRIPTOR sd = nullptr;
    const wchar_t *read_denied = L"D:P(D;;0x1;;;WD)(A;;FA;;;WD)";

    if (!last || !on)
        return;
    if (ConvertStringSecurityDescriptorToSecurityDescriptorW(read_denied,
        SDDL_REVISION_1, &sd, nullptr))
        SetFileSecurityW(chain, DACL_SECURITY_INFORMATION, sd);
    LocalFree(sd);
}
#else
static void make_chain(const char *base)
{
    int at = open(base, O_RDONLY | O_DIRECTORY);
    int next;

    for (int i = 0; i < LEVELS; i++) {
        mkdirat(at, level, 0777);
        next = openat(at, level, O_RDONLY | O_DIRECTORY);
        close(at);
        at = next;
    }
    next = openat(at, file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (write(next, TEXT, sizeof(TEXT) - 1) < 0)
        printf("folders: the deep file is not written\n");
    close(next);
    close(at);
}

static void remove_chain(const char *base)
{
    int fds[LEVELS + 1];

    fds[0] = open(base, O_RDONLY | O_DIRECTORY);
    for (int i = 1; i <= LEVELS; i++)
        fds[i] = openat(fds[i - 1], level, O_RDONLY | O_DIRECTORY);
    unlinkat(fds[LEVELS], file, 0);
    for (int i = LEVELS; i > 0; i--) {
        close(fds[i]);
        unlinkat(fds[i - 1], level, AT_REMOVEDIR);
    }
    close(fds[0]);
}

static void lock(const char *base, bool last, bool on)
{
    int at = open(base, O_RDONLY | O_DIRECTORY);
    int next;

    for (int i = 1; i < (last ? LEVELS : LEVELS / 2); i++) {
        next = openat(at, level, O_RDONLY | O_DIRECTORY);
        close(at);
        at = next;
    }
    fchmodat(at, level, on ? 0111 : 0755, 0);
    close(at);
}
#endif

static char *join_chain(const char *base)
{
    size_t size = strlen(base) + LEVELS * sizeof(level) + 1;
    char *path = malloc(size);
    size_t at = 0;

    if (!path)
        return (nullptr);
    at = (size_t)snprintf(path, size, "%s", base);
    for (int i = 0; i < LEVELS; i++)
        at += (size_t)snprintf(path + at, size - at, "%s%s",
            at ? "/" : "", level);
    return (path);
}

static bool lists_file(os_folder_t *f)
{
    bool is_folder = true;
    const char *name = os_folder_read_entry(f, &is_folder);

    return (name && !strcmp(name, file) && !is_folder
        && !os_folder_read_entry(f, &is_folder));
}

static bool read_deep(const char *path, int *refused, char *text)
{
    os_folder_t *f = nullptr;
    const char *bytes = nullptr;
    bool listed = false;
    size_t len = 0;

    for (int i = 0; i < TRIES && !f; i++) {
        f = os_folder_open(path);
        *refused += !f;
    }
    if (!f)
        return (printf("deep: the folder does not open\n"), false);
    listed = lists_file(f);
    for (int i = 0; i < TRIES && listed && !bytes; i++) {
        bytes = os_folder_read_file(f, file, &len);
        *refused += !bytes;
    }
    if (bytes)
        snprintf(text, 32, "%zu bytes: %s", len, bytes);
    else
        printf("deep: the folder does not list and read its one file\n");
    os_folder_close(f);
    return (bytes != nullptr);
}

static void read_joined(const char *whole)
{
    size_t size = strlen(whole) + sizeof(file) + 1;
    char *path = malloc(size);
    FILE *opened = nullptr;

    if (path) {
        snprintf(path, size, "%s/%s", whole, file);
        opened = fopen(path, "rb");
    }
    printf("joined: %s the file\n", opened ? "read" : "cannot read");
    if (opened)
        fclose(opened);
    free(path);
}

static int read_locked(const char *base, const char *whole, bool last)
{
    char text[32] = "";
    int refused = 0;
    bool read;

    lock(base, last, true);
    read = read_deep(whole, &refused, text);
    lock(base, last, false);
    if (read)
        printf("search only: read %s\n", text);
    return (!read);
}

static bool lists_folder(const char *path, const char *want)
{
    os_folder_t *f = os_folder_open(path);
    bool is_folder = false;
    const char *name = f ? os_folder_read_entry(f, &is_folder) : nullptr;
    bool found = false;

    while (name && !found) {
        found = !want || (!strcmp(name, want) && is_folder);
        name = os_folder_read_entry(f, &is_folder);
    }
    os_folder_close(f);
    return (found);
}

static int read_slash(const char *base, const char *whole, bool as_file)
{
    size_t size = strlen(whole) + sizeof(file) + 3;
    char *top = malloc(strlen(base) + 2);
    char *deep = malloc(size);
    char text[32] = "";
    int refused = 0;
    bool listed = false;
    bool read = false;

    if (top && deep) {
        sprintf(top, "%s/", base);
        if (as_file)
            snprintf(deep, size, "%s/%s/", whole, file);
        else
            snprintf(deep, size, "%s/", whole);
        listed = lists_folder(top, level);
        read = read_deep(deep, &refused, text);
    }
    if (listed && read)
        printf("slash: listed, and read %s\n", text);
    free(top);
    free(deep);
    return (!(listed && read));
}

static int read_root(const char *base, bool missing)
{
    char root[4] = "/";
    char *path = malloc(strlen(base) + sizeof("/missing/"));
    bool listed = false;

    if (base[0] && base[1] == ':')
        snprintf(root, sizeof(root), "%c:/", base[0]);
    if (path) {
        sprintf(path, "%s/missing/", base);
        listed = lists_folder(missing ? path : root, nullptr);
    }
    if (listed)
        printf("root: listed\n");
    free(path);
    return (!listed);
}

static int run_mode(const char *mode, const char *base, const char *whole,
    const char *relative)
{
    char text[32] = "";
    int refused = 0;

    if (!strcmp(mode, "joined"))
        return (read_joined(whole), 1);
    if (!strcmp(mode, "current") || !strcmp(mode, "base")) {
        if (os_folder_set(!strcmp(mode, "current") ? whole : base))
            return (printf("current: refused\n"), 0);
        return (printf("current: set\n"), 0);
    }
    if (!strcmp(mode, "middle") || !strcmp(mode, "last"))
        return (read_locked(base, whole, !strcmp(mode, "last")));
    if (!strcmp(mode, "slash") || !strcmp(mode, "slash-file"))
        return (read_slash(base, whole, !strcmp(mode, "slash-file")));
    if (!strcmp(mode, "root") || !strcmp(mode, "root-missing"))
        return (read_root(base, !strcmp(mode, "root-missing")));
    if (!read_deep(whole, &refused, text) || os_folder_set(base)
        || !read_deep(relative, &refused, text))
        return (1);
    printf("deep: %d refused, read %s\n", refused, text);
    return (0);
}

static bool known(const char *mode)
{
    const char *modes[] = {"deep", "joined", "current", "base", "middle",
        "last", "slash", "slash-file", "root", "root-missing"};

    for (size_t i = 0; i < sizeof(modes) / sizeof(*modes); i++)
        if (!strcmp(mode, modes[i]))
            return (true);
    return (false);
}

static int run(int argc, char **argv)
{
    const char *mode = argc == 3 ? argv[2] : "";
    char *whole = nullptr;
    char *relative = nullptr;
    int status;

    if (!known(mode))
        return (printf("usage: folders <folder> <mode>\n"), 2);
    memset(level, 'd', sizeof(level) - 1);
    memset(file, 'f', sizeof(file) - 1);
    whole = join_chain(argv[1]);
    relative = join_chain("");
    if (!whole || !relative)
        return (printf("folders: no memory\n"), 1);
    make_chain(argv[1]);
    status = run_mode(mode, argv[1], whole, relative);
    remove_chain(argv[1]);
    free(whole);
    free(relative);
    return (status);
}

int main(int argc, char **argv)
{
    return (argc > 1 ? run(argc, argv) : rows_run(run));
}

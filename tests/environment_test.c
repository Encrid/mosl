/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <winsock2.h>
    #include <windows.h>
#else
    #include <arpa/inet.h>
    #include <fcntl.h>
    #include <signal.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <unistd.h>
#endif
#ifdef __linux__
    #include <sched.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "os.h"

constexpr char TOLD[] = "as The environment tells";
constexpr char LINES[] = "x\ny\n";

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

static bool same(const char *a, const char *b)
{
#ifdef _WIN32
    return (a && b && !_stricmp(a, b));
#else
    return (a && b && !strcmp(a, b));
#endif
}

static int get(char **argv)
{
    char *value = os_env_get(argv[3]);

    printf("get: %s\n", value ? value : "none");
    free(value);
    return (0);
}

static void found(const char *mode, const char *name, const char *expected)
{
    char *path = os_path_find(name);
    char *want = os_path_whole(expected);

    if (!path)
        printf("%s: none\n", mode);
    else if (same(path, want))
        printf("%s: the program of PATH\n", mode);
    else
        printf("%s: %s\n", mode, path);
    free(path);
    free(want);
}

static int find(char **argv)
{
    found("find", argv[3], argv[4]);
    return (0);
}

static int find_here(char **argv)
{
    char *bin = join(folder, "bin");

    if (!bin || os_folder_set(bin))
        printf("find-here: the folder bin is not there\n");
    found("find-here", argv[3], argv[4]);
    free(bin);
    return (0);
}

static int find_none(char **argv)
{
    char *path = os_path_find(argv[3]);

    printf("find-none: %s\n", path ? "found" : "none");
    free(path);
    return (0);
}

static int find_plain(char **argv)
{
    char *bin = join(folder, "bin");
    char *plain = join(folder, "bin/plain");
    char name[16] = "";
    char *path = nullptr;

    if (!bin || !plain)
        return (printf("find-plain: no memory\n"), 1);
    setup(os_folder_make(bin));
    if (!strcmp(argv[3], "folder"))
        setup(os_folder_make(plain));
    else
        setup(os_file_write(plain, "", 0, !strcmp(argv[3], "x")));
    path = os_path_find("plain");
    if (argv[3][0] == '-')
        snprintf(name, sizeof(name), "plain");
    else
        snprintf(name, sizeof(name), "plain-%s", argv[3]);
    printf("%s: %s\n", name, path ? "found" : "none");
    free(path);
    free(plain);
    free(bin);
    return (0);
}

static int find_ext(char **)
{
    const char *names[] = {"bin/tool", "bin/tool.BAT", "bin/tool.EXE"};
    const char *ext = getenv("PATHEXT");
    char *bin = join(folder, "bin");
    char *want = nullptr;
#ifdef _WIN32
    char first[8] = "";

    if (ext)
        snprintf(first, sizeof(first), "%.*s", (int)strcspn(ext, ";"), ext);
#else
    const char *first = "";
#endif

    setup(os_folder_make(bin));
    for (size_t i = 0; i < 3; i++) {
        free(want);
        want = join(folder, names[i]);
        setup(os_file_write(want, "", 0, true));
    }
    free(want);
    want = malloc(strlen(bin) + 16);
    if (want)
        sprintf(want, "%s/tool%s", bin, ext ? first : "");
    found("find-ext", "tool", want ? want : "");
    free(want);
    free(bin);
    return (0);
}

static int program(char **argv)
{
    char *path = os_get_program_path();
    char *want = os_path_whole(argv[3]);

    if (same(path, want))
        printf("program: the path of the program\n");
    else
        printf("program: %s\n", path ? path : "none");
    free(path);
    free(want);
    return (0);
}

static uint32_t limited(void)
{
#ifdef __linux__
    cpu_set_t one;

    CPU_ZERO(&one);
    CPU_SET(0, &one);
    sched_setaffinity(0, sizeof(one), &one);
    return (1);
#elif defined(_WIN32)
    SetProcessAffinityMask(GetCurrentProcess(), 1);
    return (1);
#else
    return ((uint32_t)sysconf(_SC_NPROCESSORS_ONLN));
#endif
}

static uint32_t get_free_count(void)
{
#ifdef __linux__
    cpu_set_t set;

    CPU_ZERO(&set);
    sched_getaffinity(0, sizeof(set), &set);
    return ((uint32_t)CPU_COUNT(&set));
#elif defined(_WIN32)
    return ((uint32_t)GetActiveProcessorCount(ALL_PROCESSOR_GROUPS));
#else
    return ((uint32_t)sysconf(_SC_NPROCESSORS_ONLN));
#endif
}

static int cpus_free(char **)
{
    uint32_t want = get_free_count();
    uint32_t count = os_get_processor_count();

    if (count == want)
        printf("cpus-free: %s\n", TOLD);
    else
        printf("cpus-free: %u\n", count);
    return (0);
}

static int cpus(char **)
{
    uint32_t want = limited();
    uint32_t count = os_get_processor_count();

    if (count == want)
        printf("cpus: %s\n", TOLD);
    else
        printf("cpus: %u\n", count);
    return (0);
}

#ifdef _WIN32
static void *output_to(const char *path)
{
    HANDLE file = CreateFileA(path, GENERIC_READ | GENERIC_WRITE, 0, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

    SetStdHandle(STD_OUTPUT_HANDLE, file);
    return (file);
}

static void output_back(void *file)
{
    CloseHandle(file);
}
#else
static void *output_to(const char *path)
{
    int file = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    int *kept = malloc(sizeof(int));

    if (kept)
        *kept = dup(STDOUT_FILENO);
    dup2(file, STDOUT_FILENO);
    close(file);
    return (kept);
}

static void output_back(void *kept)
{
    dup2(*(int *)kept, STDOUT_FILENO);
    close(*(int *)kept);
    free(kept);
}
#endif

static int out(char **argv)
{
    char *path = join(folder, "out.txt");
    const char *text = !strcmp(argv[3], "lines") ? LINES : "x\n";
    void *kept = path ? output_to(path) : nullptr;
    int wrote = kept ? os_output_write(text, strlen(text)) : -1;
    os_folder_t *f = nullptr;
    size_t len = 0;
    char *bytes = nullptr;

    if (kept)
        output_back(kept);
    f = os_folder_open(folder);
    bytes = f ? os_folder_read_file(f, "out.txt", &len) : nullptr;
    if (!wrote && bytes && len == strlen(LINES) && !memcmp(bytes, LINES, len))
        fprintf(stderr, "out: as The output tells\n");
    else
        fprintf(stderr, "out: %d, %zu bytes\n", wrote, len);
    os_folder_close(f);
    free(path);
    return (0);
}

#ifndef _WIN32
static int block_pipe(void)
{
    sigset_t pipe_only;

    sigemptyset(&pipe_only);
    sigaddset(&pipe_only, SIGPIPE);
    pthread_sigmask(SIG_BLOCK, &pipe_only, nullptr);
    raise(SIGPIPE);
    return (0);
}

static void report_pending(int wrote)
{
    sigset_t waiting;

    sigemptyset(&waiting);
    sigpending(&waiting);
    fprintf(stderr, "out-closed: %d, pending %d\n", wrote,
        sigismember(&waiting, SIGPIPE) == 1);
}
#endif

static int out_closed(char **argv)
{
    bool closed = !strcmp(argv[3], "closed") || !strcmp(argv[3], "catch")
        || !strcmp(argv[3], "pending");
    bool none = !strcmp(argv[3], "none");
    bool catchit = !strcmp(argv[3], "catch");
    [[maybe_unused]] bool pending = !strcmp(argv[3], "pending");
    int wrote = 0;
#ifdef _WIN32
    HANDLE pipe[2] = {};

    CreatePipe(&pipe[0], &pipe[1], nullptr, 0);
    if (closed)
        CloseHandle(pipe[0]);
    SetStdHandle(STD_OUTPUT_HANDLE, none ? nullptr : pipe[1]);
    if (catchit)
        setup(os_signal_catch("caught\n"));
    wrote = os_output_write("x", 1);
#else
    int pipe_ends[2] = {-1, -1};
    int kept = dup(STDOUT_FILENO);

    if (pipe(pipe_ends))
        return (fprintf(stderr, "out-closed: no pipe\n"), 1);
    if (pending)
        block_pipe();
    if (closed)
        close(pipe_ends[0]);
    dup2(pipe_ends[1], STDOUT_FILENO);
    if (none)
        close(STDOUT_FILENO);
    if (catchit)
        setup(os_signal_catch("caught\n"));
    wrote = os_output_write("x", 1);
    dup2(kept, STDOUT_FILENO);
    if (pending) {
        report_pending(wrote);
        return (0);
    }
#endif
    return (fprintf(stderr, "out-closed: %d\n", wrote), 0);
}

static int console(char **)
{
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO info = {};
    HANDLE screen = nullptr;
    wchar_t shown[2] = {};
    DWORD count = 0;
    int wrote = 0;

    typedef HRESULT (WINAPI *alloc_t)(void *options, int *result);
    struct { int mode; BOOL use_show; WORD show; } hidden = {2, FALSE, 0};
    alloc_t alloc = (alloc_t)(void *)GetProcAddress(
        GetModuleHandleW(L"kernel32.dll"), "AllocConsoleWithOptions");
    int result = 0;

    FreeConsole();
    if (!alloc || alloc(&hidden, &result) < 0)
        AllocConsole();
    screen = CreateFileW(L"CONOUT$", GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
    SetStdHandle(STD_OUTPUT_HANDLE, screen);
    GetConsoleScreenBufferInfo(screen, &info);
    wrote = os_output_write("", 0) | os_output_write("\xc3\xa9", 2);
    ReadConsoleOutputCharacterW(screen, shown, 1, info.dwCursorPosition,
        &count);
    if (!wrote && count == 1 && shown[0] == 0x00E9)
        return (fprintf(stderr, "console: as The output tells\n"), 0);
    return (fprintf(stderr, "console: %d %u\n", wrote, (unsigned)shown[0]), 0);
#else
    void *kept = output_to("/dev/null");
    int wrote = kept ? os_output_write("\xc3\xa9", 2) : -1;

    if (kept)
        output_back(kept);
    if (!wrote)
        return (printf("console: as The output tells\n"), 0);
    return (printf("console: %d\n", wrote), 0);
#endif
}

static int port(char **)
{
    int free_port = os_get_free_port();
    struct sockaddr_in at = {};
    bool bound = false;
#ifdef _WIN32
    WSADATA data;
    SOCKET s = INVALID_SOCKET;

    WSAStartup(MAKEWORD(2, 2), &data);
    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
    int s = socket(AF_INET, SOCK_STREAM, 0);
#endif

    at.sin_family = AF_INET;
    at.sin_port = htons((uint16_t)free_port);
    at.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    bound = free_port > 0
        && !bind(s, (struct sockaddr *)&at, sizeof(at));
#ifdef _WIN32
    closesocket(s);
    WSACleanup();
#else
    close(s);
#endif
    if (bound)
        return (printf("port: free\n"), 0);
    return (printf("port: %d\n", free_port), 0);
}

constexpr int TRIES = 9;

static bool try_get([[maybe_unused]] int attempt)
{
    char *value = os_env_get("MOSL_TEST");

    free(value);
    return (value != nullptr);
}

static bool try_find([[maybe_unused]] int attempt)
{
    char *path = os_path_find("environment");

    free(path);
    return (path != nullptr);
}

static bool try_program([[maybe_unused]] int attempt)
{
    char *path = os_get_program_path();

    free(path);
    return (path != nullptr);
}

static bool try_port([[maybe_unused]] int attempt)
{
    return (os_get_free_port() > 0);
}

static bool try_cpus([[maybe_unused]] int attempt)
{
    return (os_get_processor_count() > 0);
}

static bool try_set([[maybe_unused]] int attempt)
{
    return (!os_folder_set(folder));
}

typedef struct {
    const char *name;
    bool (*run)(int attempt);
} try_t;

static const try_t TRIED[] = {
    {"get", try_get}, {"find", try_find}, {"program", try_program},
    {"port", try_port}, {"cpus", try_cpus}, {"set", try_set},
};

static int again(char **argv)
{
    const try_t *chosen = nullptr;
    int refused = 0;

    for (size_t i = 0; i < sizeof(TRIED) / sizeof(TRIED[0]); i++)
        chosen = strcmp(argv[3], TRIED[i].name) ? chosen : &TRIED[i];
    if (!chosen)
        return (printf("usage: environment <folder> again <call>\n"), 2);
    while (refused < TRIES && !chosen->run(refused))
        refused++;
    if (refused < TRIES)
        printf("%s: %d refused, then made\n", argv[3], refused);
    else
        printf("%s: always refused\n", argv[3]);
    return (0);
}

typedef struct {
    const char *name;
    int count;
    int (*run)(char **argv);
} test_mode_t;

static const test_mode_t MODES[] = {
    {"get", 4, get}, {"find", 5, find}, {"find-here", 5, find_here},
    {"find-none", 4, find_none},
    {"find-plain", 4, find_plain}, {"find-ext", 3, find_ext},
    {"program", 4, program}, {"cpus", 3, cpus}, {"cpus-free", 3, cpus_free}, {"out", 4, out},
    {"out-closed", 4, out_closed}, {"console", 3, console}, {"port", 3, port},
    {"again", 4, again},
};

int main(int argc, char **argv)
{
    if (argc < 3)
        return (printf("usage: environment <folder> <mode> [arguments]\n"), 2);
    folder = argv[1];
    for (size_t i = 0; i < sizeof(MODES) / sizeof(MODES[0]); i++) {
        if (!strcmp(argv[2], MODES[i].name) && argc == MODES[i].count)
            return (MODES[i].run(argv));
    }
    return (printf("usage: environment <folder> <mode> [arguments]\n"), 2);
}

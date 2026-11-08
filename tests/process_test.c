/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <fcntl.h>
    #include <io.h>
    #include <windows.h>
#else
    #include <signal.h>
    #include <sys/time.h>
    #include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "os.h"

constexpr size_t TEXT = 256;
constexpr size_t STACK = 1u << 20;
constexpr int TRIES = 9;
constexpr uint64_t BRIEF = 1'000'000;

typedef struct {
    const char *name;
    int count;
    int (*run)(char **argv);
} test_mode_t;

typedef struct {
    char out[TEXT];
    char err[TEXT];
    int last;
} reads_t;

typedef struct {
    os_process_t *p;
    size_t len;
    int result;
} job_t;

static const char *self;

static int child_say(char **argv)
{
    printf("%s\n", argv[3]);
    return (fflush(stdout) != 0);
}

static int child_mix(char **)
{
    fputs("a", stdout);
    fflush(stdout);
    fputs("b", stderr);
    fflush(stderr);
    fputs("c", stdout);
    return (fflush(stdout) != 0);
}

static int child_drain(char **)
{
    char chunk[4096];
    size_t total = 0;
    size_t got = 0;

#ifdef _WIN32
    _setmode(_fileno(stdin), _O_BINARY);
#endif
    do {
        got = fread(chunk, 1, sizeof(chunk), stdin);
        total += got;
    } while (got);
    printf("%zu\n", total);
    return (0);
}

static int child_hold(char **)
{
    while (fgetc(stdin) != EOF)
        continue;
    return (0);
}

static int child_stall(char **)
{
    for (;;) {
#ifdef _WIN32
        Sleep(INFINITE);
#else
        pause();
#endif
    }
    return (0);
}

static int child_exit(char **argv)
{
    return (atoi(argv[3]));
}

static int child_env(char **argv)
{
    const char *value = getenv(argv[3]);

    printf("%s\n", value ? value : "none");
    return (0);
}

static const test_mode_t CHILDREN[] = {
    {"say", 4, child_say}, {"mix", 3, child_mix}, {"drain", 3, child_drain},
    {"hold", 3, child_hold}, {"stall", 3, child_stall},
    {"exit", 4, child_exit}, {"env", 4, child_env},
};

static int run_child(int argc, char **argv)
{
    for (size_t i = 0; i < sizeof(CHILDREN) / sizeof(CHILDREN[0]); i++) {
        if (!strcmp(argv[2], CHILDREN[i].name) && argc == CHILDREN[i].count)
            return (CHILDREN[i].run(argv));
    }
    return (2);
}

static os_process_t *start_with(const os_process_start_t *start,
    const char *what, const char *word)
{
    const char *argv[5] = {self, "child", what, word, nullptr};
    os_process_start_t mine = *start;

    if (!mine.argv)
        mine.argv = argv;
    return (os_process_start(&mine));
}

static os_process_t *start_child(const char *what, const char *word)
{
    os_process_start_t start = {};

    return (start_with(&start, what, word));
}

static void add_text(char *to, const char *bytes, size_t len)
{
    size_t at = strlen(to);

    if (at + len >= TEXT)
        len = TEXT - at - 1;
    memcpy(to + at, bytes, len);
    to[at + len] = '\0';
    while (at + len > 0 && (to[at + len - 1] == '\n'
        || to[at + len - 1] == '\r'))
        to[at + --len] = '\0';
}

static void read_all(os_process_t *p, reads_t *reads)
{
    const char *bytes = nullptr;
    size_t len = 0;

    reads->last = os_process_read(p, OS_FOREVER, &bytes, &len);
    while (reads->last == OS_OUTPUT || reads->last == OS_ERRORS) {
        add_text(reads->last == OS_OUTPUT ? reads->out : reads->err, bytes,
            len);
        reads->last = os_process_read(p, OS_FOREVER, &bytes, &len);
    }
}

static int64_t end_process(os_process_t *p)
{
    int64_t status = -1000;

    if (os_process_wait(p, OS_FOREVER, &status))
        status = -1001;
    os_process_free(p);
    return (status);
}

static void read_child(const os_process_start_t *start, const char *what,
    const char *word, reads_t *reads)
{
    os_process_t *p = start_with(start, what, word);

    if (!p) {
        snprintf(reads->out, TEXT, "none");
        return;
    }
    read_all(p, reads);
    end_process(p);
}

static int mode_named(char **argv)
{
    const char *args[] = {argv[3], "child", "say", "hi", nullptr};
    os_process_start_t start = {.argv = args};
    reads_t reads = {};

    if (strcmp(argv[4], "-"))
        start.folder = argv[4];
    read_child(&start, nullptr, nullptr, &reads);
    return (printf("named: %s\n", reads.out), 0);
}

static int mode_env(char **argv)
{
    const char *env[] = {"MOSL_A=1", nullptr};
    const char *unset[] = {"MOSL_B", nullptr};
    os_process_start_t start = {.env = env};
    reads_t a = {};
    reads_t b = {};

    if (!strcmp(argv[3], "unset"))
        start.unset = unset;
    read_child(&start, "env", "MOSL_A", &a);
    read_child(&start, "env", "MOSL_B", &b);
    return (printf("env: %s %s\n", a.out, b.out), 0);
}

static int mode_streams(char **argv)
{
    os_process_start_t start = {.errors_apart = !strcmp(argv[3], "apart")};
    reads_t reads = {};

    read_child(&start, "mix", nullptr, &reads);
    return (printf("streams: out=%s err=%s\n", reads.out, reads.err), 0);
}

static int mode_full(char **argv)
{
    size_t len = (size_t)atoll(argv[3]);
    char *bytes = calloc(len, 1);
    os_process_t *p = start_child("drain", nullptr);
    reads_t reads = {};
    int wrote = -9;

    if (!bytes || !p)
        return (printf("full: no start\n"), 1);
    wrote = os_process_write(p, bytes, len);
    os_process_close(p, OS_INPUT);
    read_all(p, &reads);
    end_process(p);
    free(bytes);
    return (printf("full: %d %s\n", wrote, reads.out), 0);
}

static int mode_closed(char **argv)
{
    bool ended = !strcmp(argv[3], "exit");
    os_process_t *p = start_child(argv[3], ended ? "0" : nullptr);
    int64_t status = 0;
    int wrote = -9;

    if (!p)
        return (printf("closed: no start\n"), 1);
    if (ended && os_process_wait(p, OS_FOREVER, &status))
        printf("closed: no wait\n");
    wrote = os_process_write(p, "x", 1);
    os_process_close(p, OS_INPUT);
    end_process(p);
    return (printf("closed: %d\n", wrote), 0);
}

static int mode_timeout(char **argv)
{
    bool say = !strcmp(argv[3], "say");
    os_process_t *p = start_child(argv[3], say ? "hi" : nullptr);
    uint64_t nanos = (uint64_t)atoll(argv[4]);
    const char *bytes = nullptr;
    size_t len = 0;
    int got = -9;

    if (!p)
        return (printf("timeout: no start\n"), 1);
    got = os_process_read(p, nanos, &bytes, &len);
    os_process_kill(p);
    end_process(p);
    return (printf("timeout: %d\n", got), 0);
}

static int mode_status(char **argv)
{
    os_process_t *p = start_child("exit", argv[3]);

    if (!p)
        return (printf("status: no start\n"), 1);
    return (printf("status: %lld\n", (long long)end_process(p)), 0);
}

static int mode_kill(char **argv)
{
    os_process_t *p = start_child("hold", nullptr);

    if (!p)
        return (printf("kill: no start\n"), 1);
    if (!strcmp(argv[3], "kill"))
        os_process_kill(p);
    else
        os_process_close(p, OS_INPUT);
    return (printf("kill: %lld\n", (long long)end_process(p)), 0);
}

static void read_job(void *arg)
{
    job_t *job = arg;
    const char *bytes = nullptr;
    size_t len = 0;

    job->result = os_process_read(job->p, OS_FOREVER, &bytes, &len);
}

static void write_job(void *arg)
{
    job_t *job = arg;
    char *bytes = calloc(job->len, 1);

    job->result = bytes ? os_process_write(job->p, bytes, job->len) : -9;
    free(bytes);
}

static int run_job(const char *mode, char **argv, os_thread_fn_t fn)
{
    bool say = !strcmp(argv[3], "say");
    bool kill = !say && strcmp(argv[3], "drain");
    job_t job = {.p = start_child(argv[3], say ? "hi" : nullptr),
        .len = 1u << 22, .result = -9};
    os_thread_t *t = job.p ? os_thread_start(fn, &job, STACK) : nullptr;

    if (!t)
        return (printf("%s: no start\n", mode), 1);
    if (kill)
        os_process_kill(job.p);
    os_thread_join(t);
    os_process_close(job.p, OS_INPUT);
    end_process(job.p);
    return (printf("%s: %d\n", mode, job.result), 0);
}

static int mode_kill_read(char **argv)
{
    return (run_job("kill-read", argv, read_job));
}

static int mode_kill_write(char **argv)
{
    return (run_job("kill-write", argv, write_job));
}

static int mode_interrupt(char **argv)
{
    os_process_start_t start = {.signals = true,
        .ignore_interrupt = !strcmp(argv[3], "ignore")};
    os_process_t *p = start_with(&start, "hold", nullptr);
    int64_t status = 0;
    int waited = -9;

    if (!p)
        return (printf("interrupt: no start\n"), 1);
    if (os_process_signal(p, OS_INTERRUPT))
        printf("interrupt: no signal\n");
    waited = os_process_wait(p, 500 * BRIEF, &status);
    os_process_kill(p);
    end_process(p);
    printf("interrupt: %s\n", waited == 1 ? "ignored" : "default");
    return (0);
}

static int mode_say(char **argv)
{
    os_process_start_t start = {};
    reads_t reads = {};

    read_child(&start, "say", argv[3], &reads);
    return (printf("say: [%s]\n", reads.out), 0);
}

static int get_which(const char *name)
{
    if (!strcmp(name, "interrupt"))
        return (OS_INTERRUPT);
    if (!strcmp(name, "termination"))
        return (OS_TERMINATION);
    return (OS_HANG_UP);
}

static os_process_t *start_signals(bool signals)
{
    os_process_start_t start = {.signals = signals};

    return (start_with(&start, "hold", nullptr));
}

static int mode_signal(char **argv)
{
    os_process_t *p = start_signals(true);
    int sent = -9;

    if (!p)
        return (printf("signal: no start\n"), 1);
    sent = os_process_signal(p, get_which(argv[3]));
    if (sent)
        os_process_kill(p);
    printf("signal: %d %lld\n", sent, (long long)end_process(p));
    return (0);
}

static int mode_nosignal(char **argv)
{
    os_process_t *p = start_signals(!strcmp(argv[3], "on"));
    int sent = -9;

    if (!p)
        return (printf("nosignal: no start\n"), 1);
    sent = os_process_signal(p, OS_TERMINATION);
    os_process_kill(p);
    end_process(p);
    return (printf("nosignal: %d\n", sent), 0);
}

static int mode_hangup(char **argv)
{
    os_process_t *p = start_signals(true);
    int first = -9;
    int second = -9;

    if (!p)
        return (printf("hangup: no start\n"), 1);
    first = os_process_signal(p, get_which(argv[3]));
    second = os_process_signal(p, OS_INTERRUPT);
    os_process_kill(p);
    end_process(p);
    return (printf("hangup: %d %d\n", first, second), 0);
}

static int mode_free(char **argv)
{
    os_process_t *p = nullptr;

    if (!strcmp(argv[3], "hold"))
        p = start_child("hold", nullptr);
    os_process_free(p);
    return (printf("free: %s\n", p ? "done" : "none"), 0);
}

static int mode_close(char **argv)
{
    os_process_t *p = start_child("say", "hi");
    const char *bytes = nullptr;
    size_t len = 0;
    int got = -9;

    if (!p)
        return (printf("close: no start\n"), 1);
    os_process_close(p, !strcmp(argv[3], "output") ? OS_OUTPUT : OS_INPUT);
    got = os_process_read(p, OS_FOREVER, &bytes, &len);
    os_process_kill(p);
    end_process(p);
    return (printf("close: %d\n", got), 0);
}

static int mode_wait(char **argv)
{
    bool hold = !strcmp(argv[3], "hold");
    os_process_t *p = start_child(argv[3], hold ? nullptr : "0");
    int64_t status = 0;
    int waited = -9;

    if (!p)
        return (printf("wait: no start\n"), 1);
    waited = os_process_wait(p, hold ? BRIEF : OS_FOREVER, &status);
    os_process_kill(p);
    end_process(p);
    return (printf("wait: %d\n", waited), 0);
}

static int mode_write_after(char **argv)
{
    os_process_t *p = start_child("hold", nullptr);
    int wrote = -9;

    if (!p)
        return (printf("write-after: no start\n"), 1);
    if (!strcmp(argv[3], "input")) {
        os_process_close(p, OS_INPUT);
        os_process_close(p, OS_INPUT);
    }
    wrote = os_process_write(p, "x", 1);
    os_process_kill(p);
    end_process(p);
    return (printf("write-after: %d\n", wrote), 0);
}

#ifndef _WIN32
static volatile sig_atomic_t alarms;

static void count_alarm(int)
{
    alarms = alarms + 1;
}

static void start_alarms(void)
{
    struct sigaction act = {.sa_handler = count_alarm};
    struct itimerval every = {{0, 20000}, {0, 20000}};

    sigemptyset(&act.sa_mask);
    sigaction(SIGALRM, &act, nullptr);
    setitimer(ITIMER_REAL, &every, nullptr);
}

static void stop_alarms(void)
{
    struct itimerval none = {};

    setitimer(ITIMER_REAL, &none, nullptr);
}

static void hold_alarms(bool hold)
{
    sigset_t only;

    sigemptyset(&only);
    sigaddset(&only, SIGALRM);
    pthread_sigmask(hold ? SIG_BLOCK : SIG_UNBLOCK, &only, nullptr);
}
#else
static void start_alarms(void)
{
}

static void stop_alarms(void)
{
}

static void hold_alarms(bool)
{
}
#endif

static void kill_later(void *arg)
{
    os_event_t *never = os_event_create();

    if (never)
        os_event_wait(never, 200 * BRIEF);
    os_event_free(never);
    os_process_kill(arg);
}

static int interrupt_write(os_process_t *p)
{
    char *bytes = calloc(1u << 22, 1);
    os_thread_t *t = nullptr;
    int wrote = -9;

    hold_alarms(true);
    t = bytes ? os_thread_start(kill_later, p, STACK) : nullptr;
    hold_alarms(false);
    start_alarms();
    if (t)
        wrote = os_process_write(p, bytes, 1u << 22);
    stop_alarms();
    os_thread_join(t);
    free(bytes);
    return (wrote);
}

static int mode_interrupted(char **argv)
{
    bool say = !strcmp(argv[4], "say");
    os_process_t *p = start_child(argv[4], say ? "hi" : nullptr);
    const char *bytes = nullptr;
    size_t len = 0;
    int64_t status = 0;
    int got = -9;

    if (!p)
        return (printf("interrupted: no start\n"), 1);
    if (!strcmp(argv[3], "write"))
        got = interrupt_write(p);
    start_alarms();
    if (!strcmp(argv[3], "read"))
        got = os_process_read(p, 200 * BRIEF, &bytes, &len);
    if (!strcmp(argv[3], "wait"))
        got = os_process_wait(p, 200 * BRIEF, &status);
    stop_alarms();
    os_process_kill(p);
    end_process(p);
    return (printf("interrupted: %d\n", got), 0);
}

static int mode_late_kill(char **argv)
{
    os_process_t *p = start_child("exit", argv[3]);
    int64_t status = -1;

    if (!p)
        return (printf("late-kill: no start\n"), 1);
    if (os_process_wait(p, OS_FOREVER, &status))
        printf("late-kill: no wait\n");
    os_process_kill(p);
    return (printf("late-kill: %lld\n", (long long)end_process(p)), 0);
}

static int mode_read_after(char **argv)
{
    os_process_t *p = start_child("exit", argv[3]);
    const char *bytes = nullptr;
    size_t len = 0;
    int64_t status = 0;
    int got = -9;

    if (!p)
        return (printf("read-after: no start\n"), 1);
    if (os_process_wait(p, OS_FOREVER, &status))
        printf("read-after: no wait\n");
    got = os_process_read(p, OS_FOREVER, &bytes, &len);
    end_process(p);
    return (printf("read-after: %d %lld\n", got, (long long)status), 0);
}

static bool try_once(bool signals)
{
    const char *env[] = {"MOSL_E=1", nullptr};
    const char *unset[] = {"MOSL_F", nullptr};
    os_process_start_t start = {.errors_apart = true, .env = env,
        .unset = unset, .signals = signals};
    reads_t reads = {};

    read_child(&start, "say", "hi", &reads);
    return (!strcmp(reads.out, "hi"));
}

static int mode_again(char **argv)
{
    int refused = 0;

    while (refused < TRIES && !try_once(!strcmp(argv[3], "signals")))
        refused++;
    if (refused < TRIES)
        printf("again: %d refused, then made\n", refused);
    else
        printf("again: always refused\n");
    return (0);
}

static const test_mode_t MODES[] = {
    {"named", 5, mode_named}, {"env", 4, mode_env},
    {"streams", 4, mode_streams}, {"full", 4, mode_full},
    {"closed", 4, mode_closed}, {"timeout", 5, mode_timeout},
    {"status", 4, mode_status}, {"kill", 4, mode_kill},
    {"kill-read", 4, mode_kill_read}, {"kill-write", 4, mode_kill_write},
    {"interrupt", 4, mode_interrupt}, {"signal", 4, mode_signal},
    {"nosignal", 4, mode_nosignal}, {"hangup", 4, mode_hangup},
    {"free", 4, mode_free}, {"close", 4, mode_close},
    {"wait", 4, mode_wait}, {"again", 4, mode_again},
    {"read-after", 4, mode_read_after},
    {"write-after", 4, mode_write_after},
    {"interrupted", 5, mode_interrupted},
    {"late-kill", 4, mode_late_kill}, {"say", 4, mode_say},
};

int main(int argc, char **argv)
{
    self = argv[0];
    if (argc >= 3 && !strcmp(argv[1], "child"))
        return (run_child(argc, argv));
    for (size_t i = 0; argc >= 3 && i < sizeof(MODES) / sizeof(MODES[0]);
        i++) {
        if (!strcmp(argv[2], MODES[i].name) && argc == MODES[i].count)
            return (MODES[i].run(argv));
    }
    return (printf("usage: process <folder> <mode> [arguments]\n"), 2);
}

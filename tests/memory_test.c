/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "os.h"
#include "rows.h"

static size_t count_zero(const unsigned char *bytes, size_t size)
{
    size_t zero = 0;

    for (size_t i = 0; i < size; i++)
        zero += bytes[i] == 0;
    return (zero);
}

static size_t count_kept(unsigned char *bytes, size_t size)
{
    size_t kept = 0;

    for (size_t i = 0; i < size; i++)
        bytes[i] = (unsigned char)(i % 251 + 1);
    for (size_t i = 0; i < size; i++)
        kept += bytes[i] == (unsigned char)(i % 251 + 1);
    return (kept);
}

static int use_bytes(size_t reserve, size_t first, size_t grow)
{
    unsigned char *base = os_reserve(reserve, first);
    size_t size = grow > first ? grow : first;
    size_t zero;

    if (!base)
        return (printf("commit: no reservation\n"), 0);
    if (grow > first && os_commit(base, grow))
        printf("commit: refused\n");
    zero = count_zero(base, size);
    printf("commit: %zu zero, %zu kept of %zu\n", zero,
        count_kept(base, size), size);
    os_release(base, reserve);
    return (0);
}

static int touch_byte(size_t reserve, size_t first, size_t at)
{
    volatile unsigned char *base = os_reserve(reserve, first);

    if (!base)
        return (printf("touch: no reservation\n"), 0);
    printf("touch %zu\n", at);
    fflush(stdout);
    base[at] = 1;
    printf("touched %zu\n", at);
    os_release((void *)base, reserve);
    return (0);
}

static int run(int argc, char **argv)
{
    size_t sizes[3] = {};

    if (argc != 5)
        return (printf("usage: memory commit|touch <reserve> <commit> "
            "<grow|at>\n"), 2);
    for (int i = 0; i < 3; i++)
        sizes[i] = strtoull(argv[i + 2], nullptr, 10);
    if (!strcmp(argv[1], "commit"))
        return (use_bytes(sizes[0], sizes[1], sizes[2]));
    if (!strcmp(argv[1], "touch"))
        return (touch_byte(sizes[0], sizes[1], sizes[2]));
    return (printf("usage: memory commit|touch <reserve> <commit> "
        "<grow|at>\n"), 2);
}

int main(int argc, char **argv)
{
    return (argc > 1 ? run(argc, argv) : rows_run(run));
}

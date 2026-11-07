/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include "os_whole.h"

constexpr size_t MOST = 512;

static int show_root(const char *text)
{
    wchar_t whole[MOST + 1] = {};
    size_t len = strlen(text);

    if (len >= MOST)
        return (printf("usage: whole root <a whole path>\n"), 2);
    for (size_t i = 0; i < len; i++)
        whole[i] = (wchar_t)(unsigned char)text[i];
    printf("root ends at %zu\n", os_get_root_end(whole));
    return (0);
}

int main(int argc, char **argv)
{
    wchar_t whole[OS_PLACE + MOST + 1] = {};
    char out[OS_PLACE + MOST + 1] = {};
    size_t len = argc == 2 ? strlen(argv[1]) : MOST;

    if (argc == 3 && !strcmp(argv[1], "root"))
        return (show_root(argv[2]));

    if (len >= MOST)
        return (printf("usage: whole <a whole path of less than 512 bytes>\n"),
            2);
    for (size_t i = 0; i < len; i++)
        whole[OS_PLACE + i] = (wchar_t)(unsigned char)argv[1][i];
    os_mark_whole(whole, len + 1);
    for (size_t i = 0; whole[i]; i++)
        out[i] = (char)whole[i];
    printf("%s\n", out);
    return (0);
}

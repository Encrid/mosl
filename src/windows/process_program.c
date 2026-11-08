/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "os_types.h"

static char *join_folder(const os_process_start_t *start)
{
    const char *name = start->argv[0];
    bool whole = strchr("\\/", name[0]) || name[1] == ':';
    size_t size = strlen(name) + 2;
    char *path = nullptr;

    if (!whole && start->folder)
        size += strlen(start->folder);
    path = malloc(size);
    if (path && !whole && start->folder)
        snprintf(path, size, "%s\\%s", start->folder, name);
    else if (path)
        snprintf(path, size, "%s", name);
    return (path);
}

wchar_t *os_find_program(const os_process_start_t *start)
{
    char *path = nullptr;
    char *whole = nullptr;
    wchar_t *wide = nullptr;

    if (!strpbrk(start->argv[0], "\\/")) {
        whole = os_find_text(start->argv[0]);
    } else {
        path = join_folder(start);
        whole = path ? os_whole_text(path) : nullptr;
        free(path);
    }
    wide = whole ? os_widen(whole) : nullptr;
    free(whole);
    return (wide);
}

bool os_prepare_launch(const os_process_start_t *start, os_launch_t *launch)
{
    launch->program = os_find_program(start);
    launch->line = os_join_line(start->argv);
    launch->block = os_build_block(start->env, start->unset);
    if (start->folder)
        launch->folder = os_widen(start->folder);
    if (!launch->program || !launch->line || !launch->block)
        return (false);
    return (launch->folder || !start->folder);
}

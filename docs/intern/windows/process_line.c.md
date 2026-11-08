# src/windows/process_line.c

On Windows, this file joins the arguments of a process into one command line.

## The quotes

The program splits the line again by the rules of `CommandLineToArgvW`. So each argument keeps its text after the split:

- An argument with no space, tab, line end or `"`, and not empty, goes as it is.
- Else it goes between 2 `"`. Each `"` in it gets a `\\` before it.
- A run of `\\` before a `"` doubles, and a run of `\\` at the end doubles, because the rules halve such runs.

## Functions

| Function | Result |
| --- | --- |
| `needs_quotes(arg)` | Gives true if `arg` needs quotes, as *The quotes* tells. |
| `add_slashes(out, count)` | Writes `count` backslashes into `out`, and gives `count`. |
| `add_arg(out, arg)` | Writes `arg` into `out`, with quotes if it needs them, and gives the count of bytes. |
| `os_join_line(argv)` | Gives the command line in UTF-16, which the caller frees, or `nullptr`. |

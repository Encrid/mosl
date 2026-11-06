# src/windows/env.c

On Windows, `os_env_get` gives a variable of the environment in UTF-8.

## The variable

`os_get_variable` turns the name into UTF-16 with `os_widen`, and asks `GetEnvironmentVariableW` for the size of the value.
`ERROR_ENVVAR_NOT_FOUND` gives `nullptr`. Else the second call writes the value, and `os_narrow` turns it into UTF-8.

`os_env_get` gives its result, and `os_path_find` of `src/windows/find.c` calls it for `PATH` and `PATHEXT`.
Thus no call of `src` calls a function of `[leaks] memory` of `.toolchain/config/leaks.toml`, which `test --leaks` would count 2 times.

The second call must give the length of the value. Another result tells that the call failed, or that the value changed between the calls.

## Functions

| Function | Result |
| --- | --- |
| `get_variable(name)` | Gives the value in UTF-8, or `nullptr`. |
| `os_env_get(name)` | Gives the result of `os_get_variable`. |

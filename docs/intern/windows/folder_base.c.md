# src/windows/folder_base.c

On Windows, `os_folder_base` asks the shell for a known folder.
It keeps the contract of `docs/extern/os.h.md`, *The base folders*.

## The known folders

`OS_FOLDER_USER` gives `FOLDERID_SavedGames`, and `OS_FOLDER_LOCAL` gives `FOLDERID_LocalAppData`.
The file writes the 2 GUIDs of `KNOWNFOLDERID` itself. Thus it needs no library of GUIDs at the link.
`CREATE` is the value of `KF_FLAG_CREATE`, 0x00008000, because `windows.h` does not give the flags of the shell.

## The call

1. A kind that is not known gives `nullptr`, and loads no library.
2. `LoadLibraryW` loads `shell32.dll`, then `ole32.dll`.
3. `GetProcAddress` gives `SHGetKnownFolderPath` and `CoTaskMemFree`.
4. If both are there, `SHGetKnownFolderPath` gives the folder in UTF-16, and `os_narrow` turns it into UTF-8.
5. `CoTaskMemFree` frees the text of the shell, also after a failure of `SHGetKnownFolderPath`.
6. `FreeLibrary` frees each library that loaded.

A failure of a step gives `nullptr`.

## Functions

| Function | Result |
| --- | --- |
| `get_known(shell, ole, id)` | The path of the known folder `id`, as *The call* tells, or `nullptr`. |
| `os_folder_base(kind)` | The base folder of `kind`, or `nullptr`. |

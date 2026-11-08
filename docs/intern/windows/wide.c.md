# src/windows/wide.c

This file turns a text of UTF-8 into UTF-16, and back, on Windows.
Each text of the layer is UTF-8, and each call of Windows takes UTF-16.

## Functions

| Function | Result |
| --- | --- |
| `os_widen(text)` | Gives a copy of `text` in UTF-16, made with `MultiByteToWideChar`, or `nullptr`. The caller frees it. |
| `os_widen_texts(texts)` | Gives a list of copies of `texts` in UTF-16, with `nullptr` after the last, or `nullptr`. A null `texts` gives an empty list. |
| `os_free_texts(texts)` | Frees each copy and the list. A null `texts` does nothing. |
| `os_narrow(wide)` | Gives a copy of `wide` in UTF-8, made with `WideCharToMultiByte`, or `nullptr`. The caller frees it. |

# BASIC-Z-GUI

A Win32 GUI rewrite inspired by CPU-Z, written in BASICGOL.

## Build (Windows, MSVC)

```bat
cl /std:c11 /I ..\..\..\include basicz_gui.c /link comctl32.lib user32.lib gdi32.lib kernel32.lib
basicz_gui.exe
```

## Notes

- Windows only (Win32 API).
- CPU and memory tabs are populated; other tabs are placeholders.
- No installer; run the executable directly.

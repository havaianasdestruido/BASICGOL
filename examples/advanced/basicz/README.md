# BASIC-Z

A console rewrite inspired by CPU-Z, written entirely in BASICGOL.

## Build

```sh
cc -std=c11 -I ../../../include basicz.c -o basicz
./basicz
```

On Windows (MSVC):

```bat
cl /std:c11 /I ..\..\..\include basicz.c
basicz.exe
```

## Notes

- CPU, cache and memory info come from `/proc` on Linux or Win32 APIs on Windows.
- GPU detection, SPD and benchmarks are not implemented in this version.
- No GUI: this is a console tool.

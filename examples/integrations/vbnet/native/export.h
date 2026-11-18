#ifndef BASICGOL_INTEROP_EXPORT_H
#define BASICGOL_INTEROP_EXPORT_H
#if defined(_WIN32)
#  define BG_EXPORT __declspec(dllexport)
#else
#  define BG_EXPORT
#endif
#endif

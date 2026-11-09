# Test report — BASICGOL 1.0

- Language suite: 307 checks (M0 + M1 equivalence), run via `make test`.
- Library unit tests: 230 checks, via `tests/run_biblioteca.sh`.
- Integration/fatal tests: 30 checks.
- Portability matrix: 70 checks (see docs/11-matriz-compatibilidade.md).
- Compilers exercised: GCC 12.2.0, Clang via zig cc, MinGW-w64 in CI.
- Sanitizers: ASan + UBSan + LeakSanitizer (GCC/Clang).
- Limitations: MSVC and AArch64 not executed; see docs/limitations.md.
- Reproducibility: build from clean checkout with `make`; see BUILDING.md.

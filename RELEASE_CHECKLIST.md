# Release checklist — BASICGOL

- [ ] `make limpar && make` succeeds on Linux (GCC).
- [ ] `make test` passes (language + library).
- [ ] `make test-sanitize` passes (GCC/Clang).
- [ ] `make test-portabilidade` reviewed; new limitations documented.
- [ ] `make exemplos` compiles all examples.
- [ ] Examples consistent with implementation (run manually).
- [ ] README.md and README.pt.md updated (version, counts).
- [ ] BUILDING.md, feature-matrix, limitations, tutorial updated.
- [ ] No unimplemented features documented as available.
- [ ] Tag release with version (language + library).
- [ ] Publish docs site (GitHub Pages workflow).

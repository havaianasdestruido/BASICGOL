# Limitações conhecidas — BASICGOL

- Sem máquina virtual, coletor de lixo, interpretador ou compilador próprio no
  caminho crítico: o modelo de execução é o de C.
- Construtores não redutíveis a macros ficam marcados como **M1** (exigem o
  transpilador `basicgolc`) ou simplesmente não existem.
- Recursos não implementados não são documentados como disponíveis.
- MSVC é apenas avaliado, não executado na matriz de portabilidade.
- AArch64 foi compilado, não executado.
- `zig cc` é usado como frontend Clang no CI; Clang nativo não é testado
  separadamente.
- Sanitizers (ASan/UBSan/LeakSanitizer) estão disponíveis apenas em
  GCC/Clang.
- A documentação do site (Docusaurus) é opcional e requer Node.js/Python.

Consulte `docs/04-restricoes-e-decisoes.md` e `docs/11-matriz-compatibilidade.md`.

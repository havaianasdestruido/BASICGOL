// Valida os caminhos relativos no artifact final (inclusive sob um baseUrl).
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const raiz = path.resolve(__dirname, '../../website/build/playground');
for (const arquivo of ['index.html', 'tema.css', 'app.js', 'ferramentas.js']) {
  const texto = fs.readFileSync(path.join(raiz, arquivo), 'utf8');
  assert(!texto.includes('../vendor/') && !texto.includes('../publico/'), arquivo);
  for (const referencia of texto.matchAll(/["'(](\.\/(?:vendor|publico)\/[^"')]+)["')]/g)) {
    assert(fs.statSync(path.join(raiz, referencia[1])).isFile(), referencia[1]);
  }
}
assert(fs.statSync(path.join(raiz, 'publico/toolchain/wasi-cc.tar.gz')).size > 1000000);
assert(fs.existsSync(path.join(raiz, 'ATRIBUICOES.md')));
console.log('Playground publicado: interface, dependências e compilador verificados.');

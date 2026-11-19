// Publica a página independente sem alterar os caminhos usados no desenvolvimento.
const {cp, mkdir, readFile, writeFile, access} = require('node:fs/promises');
const path = require('node:path');

module.exports = async function publicarPlayground(outDir, baseUrl = '/BASICGOL/') {
  const raiz = path.resolve(__dirname, '..');
  const pacote = path.join(raiz, 'publico/toolchain/wasi-cc.tar.gz');
  try {
    await access(pacote);
  } catch {
    throw new Error('Toolchain do playground ausente. Rode npm run playground:prepare em website/.');
  }
  const destino = path.join(outDir, 'playground');
  await mkdir(destino, {recursive: true});
  await cp(path.join(raiz, 'ativo'), destino, {recursive: true});
  await cp(path.join(raiz, 'vendor'), path.join(destino, 'vendor'), {recursive: true});
  await cp(path.join(raiz, 'publico'), path.join(destino, 'publico'), {recursive: true});
  await cp(path.join(raiz, 'ATRIBUICOES.md'), path.join(destino, 'ATRIBUICOES.md'));
  for (const arquivo of ['index.html', 'tema.css', 'app.js', 'ferramentas.js']) {
    const alvo = path.join(destino, arquivo);
    let texto = await readFile(alvo, 'utf8');
    if (arquivo === 'index.html') {
      texto = texto.replace('</head>', `<link rel="describedby" href="${baseUrl}llms.txt" /></head>`);
    }
    await writeFile(alvo, texto.replaceAll('../vendor/', './vendor/').replaceAll('../publico/', './publico/'));
  }
};

#!/usr/bin/env node
/*
 * extrair-icones.js — extrai os ícones usados na interface do
 * `vendor/fa/svgs/solid/*.svg` (Font Awesome Free 5.6.3) para um JSON com o
 * `viewBox` e o `path` de cada um.
 *
 * O cplayground original carrega o Font Awesome inteiro por CDN; aqui os
 * ícones viram SVG local, uns poucos KB, e a página funciona offline.
 *
 *   node playground/tools/extrair-icones.js caminho/para/fontawesome-free
 *
 * A saída é `playground/vendor/fa/icones.json`, versionada no repositório.
 */
'use strict';

const fs = require('fs');
const path = require('path');

const RAIZ = path.resolve(__dirname, '..', '..');
const DESTINO = path.join(RAIZ, 'playground', 'vendor', 'fa', 'icones.json');

/** Ícones que a interface usa (nomes do Font Awesome 5). */
const ICONES = [
  'cog', // configurações
  'bug', // "Debug" (estado parado do botão Run)
  'stop',
  'file-code',
  'trash-alt',
  'terminal',
  'times',
  'play',
  'upload',
  'book',
  'question-circle',
  'exclamation-triangle',
  'check',
  'spinner',
  'lightbulb',
  'code',
  'keyboard',
  'download',
  'external-link-alt',
  'sync-alt',
  'clock',
];

const origem = process.argv[2] || path.join(RAIZ, 'playground', '.vendor-build', 'fontawesome-free');
if (!fs.existsSync(origem)) {
  console.error(`falta ${origem}\n\n`
    + 'Baixe o pacote do Font Awesome Free 5.6.3 e aponte o caminho:\n'
    + '  curl -o fa.tgz https://registry.npmjs.org/@fortawesome/fontawesome-free/-/fontawesome-free-5.6.3.tgz\n'
    + '  mkdir -p playground/.vendor-build && tar xzf fa.tgz -C playground/.vendor-build\n'
    + '  node playground/tools/extrair-icones.js playground/.vendor-build/package');
  process.exit(2);
}

/** Lê um SVG do Font Awesome e devolve { w, h, d }. */
function lerIcone(arquivo) {
  const svg = fs.readFileSync(arquivo, 'utf8');
  const viewBox = svg.match(/viewBox="([^"]+)"/);
  const caminho = svg.match(/<path[^>]*\sd="([^"]+)"/);
  if (!viewBox || !caminho) throw new Error(`SVG sem viewBox/path: ${arquivo}`);
  const [x, y, w, h] = viewBox[1].split(/\s+/).map(Number);
  if (x !== 0 || y !== 0) throw new Error(`viewBox deslocado em ${arquivo}: ${viewBox[1]}`);
  return { w, h, d: caminho[1] };
}

const saida = {};
const faltando = [];
for (const nome of ICONES) {
  const arquivo = path.join(origem, 'svgs', 'solid', `${nome}.svg`);
  if (!fs.existsSync(arquivo)) {
    faltando.push(nome);
    continue;
  }
  saida[nome] = lerIcone(arquivo);
}

fs.mkdirSync(path.dirname(DESTINO), { recursive: true });
fs.writeFileSync(DESTINO, `${JSON.stringify(saida, null, 1)}\n`);

const licenca = path.join(origem, 'LICENSE.txt');
if (fs.existsSync(licenca)) {
  fs.copyFileSync(licenca, path.join(path.dirname(DESTINO), 'LICENSE.txt'));
}

console.log(`pronto: ${path.relative(RAIZ, DESTINO)} (${Object.keys(saida).length} ícones, `
  + `${(fs.statSync(DESTINO).size / 1024).toFixed(1)} KB)`);
if (faltando.length) console.log(`  ausentes: ${faltando.join(', ')}`);

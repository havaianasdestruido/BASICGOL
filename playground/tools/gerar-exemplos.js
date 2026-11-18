#!/usr/bin/env node
/*
 * gerar-exemplos.js — gera `playground/ativo/exemplos.js` a partir dos
 * programas reais de `docs/exemplos/*.c`.
 *
 * O playground não inventa exemplos: ele mostra exatamente os programas da
 * documentação (os mesmos que `make exemplos` compila e os testes dourados
 * verificam). Rode este script quando um exemplo do repositório mudar.
 *
 *   node playground/tools/gerar-exemplos.js
 *
 * O título é derivado da primeira linha de comentário do arquivo, que segue o
 * padrão "// nome.c — descrição." nos exemplos do repositório.
 */
'use strict';

const fs = require('fs');
const path = require('path');

const RAIZ = path.resolve(__dirname, '..', '..');
const ORIGEM = path.join(RAIZ, 'docs', 'exemplos');
const DESTINO = path.join(RAIZ, 'playground', 'ativo', 'exemplos.js');

/**
 * Programas que o playground não consegue compilar — e por quê.
 *
 * Ambos são perfil M1: usam a forma "açucarada" do laço (`PARA i DE 1 ATE 5
 * FACA`), que exige o transpilador `basicgolc`. No perfil M0 — que é o que
 * este playground compila, só com as macros de <basicgol.h> — o laço contado
 * é escrito `PARA (i, 1, 5) { ... }` (ver docs/exemplos/tabuada.c).
 *
 * O playground avisa isso na interface quando alguém tenta usar a forma M1:
 * o erro do compilador aponta a macro e a linha, e é a mesma mensagem que o
 * `make` daria fora do navegador.
 */
const EXCLUIDOS = new Map([
  ['controle_para_acucar.c', 'perfil M1 (PARA i DE ... FACA exige basicgolc)'],
  ['para_contado.c', 'perfil M1 (PARA i DE ... FACA exige basicgolc)'],
]);

/** "// adivinha.c — REPITA/ATE_QUE, ..." -> "Adivinha: REPITA/ATE_QUE, ..." */
function extrairTitulo(fonte, arquivo) {
  const nome = arquivo.replace(/\.c$/, '');
  // Título bonito: "controle_lacos" -> "Controle de lacos"? Não: os nomes já
  // são frases curtas, então trocamos "_" por espaço e capitalizamos.
  const titulo = nome.replace(/_/g, ' ').replace(/^./, (c) => c.toUpperCase());

  const linhas = fonte.split('\n').slice(0, 16);
  for (const linha of linhas) {
    // Comentário de linha (`// x.c — ...`) ou primeira linha de um bloco
    // (` * x.c — ...`), sempre citando o próprio arquivo.
    const m = linha.match(new RegExp(`^\\s*(?://|\\*)\\s*${nome}\\.c\\s*[\u2014-]\\s*(.+)$`));
    if (!m) continue;
    return `${titulo} — ${m[1].trim().replace(/\.$/, '')}`;
  }
  return titulo;
}

const arquivos = fs.readdirSync(ORIGEM)
  .filter((f) => f.endsWith('.c') && !EXCLUIDOS.has(f))
  .sort();

const exemplos = [];
for (const arquivo of arquivos) {
  const fonte = fs.readFileSync(path.join(ORIGEM, arquivo), 'utf8');
  let entrada = null;
  const arquivoEntrada = path.join(ORIGEM, arquivo.replace(/\.c$/, '.entrada.txt'));
  if (fs.existsSync(arquivoEntrada)) {
    entrada = fs.readFileSync(arquivoEntrada, 'utf8').replace(/\r\n/g, '\n');
  }
  exemplos.push({
    arquivo,
    titulo: extrairTitulo(fonte, arquivo),
    entrada,
    fonte,
  });
}

const cabecalho = `/*
 * exemplos.js — GERADO por playground/tools/gerar-exemplos.js.
 * Não edite à mão: altere docs/exemplos/*.c e rode o script de novo.
 *
 * Cada entrada é um programa do repositório, com o texto exato do arquivo
 * \`docs/exemplos/<arquivo>.c\` e, quando existe, a entrada de
 * \`<arquivo>.entrada.txt\` (a mesma dos testes dourados).
 */

export const EXEMPLOS = [
`;

const corpo = exemplos.map(({ arquivo, titulo, entrada, fonte }) => {
  const campos = [
    `  {\n`,
    `    arquivo: ${JSON.stringify(arquivo)},\n`,
    `    titulo: ${JSON.stringify(titulo)},\n`,
  ];
  if (entrada !== null) campos.push(`    entrada: ${JSON.stringify(entrada)},\n`);
  campos.push(`    codigo: ${JSON.stringify(fonte)},\n`);
  campos.push(`  },\n`);
  return campos.join('');
}).join('');

const rodape = `];

/** Busca um exemplo pelo título (ou pelo nome do arquivo, sem .c). */
export function acharExemplo(nome) {
  const chave = String(nome || '').toLowerCase();
  return EXEMPLOS.find((e) => e.titulo.toLowerCase() === chave
    || e.arquivo.replace(/\\.c$/, '') === chave
    || e.titulo.toLowerCase().includes(chave)) || null;
}

export default EXEMPLOS;
`;

fs.writeFileSync(DESTINO, cabecalho + corpo + rodape);
for (const [arquivo, motivo] of EXCLUIDOS) {
  console.log(`  (fora: ${arquivo} — ${motivo})`);
}
console.log(`pronto: ${path.relative(RAIZ, DESTINO)} (${exemplos.length} exemplos, `
  + `${(fs.statSync(DESTINO).size / 1024).toFixed(0)} KB)`);
for (const e of exemplos) {
  console.log(`  ${e.arquivo}${e.entrada !== null ? ' (com entrada)' : ''} — ${e.titulo}`);
}

#!/usr/bin/env node
/*
 * verificar-navegador.mjs — executa **o código do navegador** (`ativo/*.js`)
 * fora do navegador, para provar que ele compila e roda BASICGOL de verdade.
 *
 * Diferente de `verificar.js` (que replica o protocolo em CommonJS), este
 * script importa `ativo/ferramentas.js` e `ativo/armazenamento.js` como
 * módulos ES, sem cópia nenhuma — é o mesmo arquivo que a página carrega. A
 * única coisa falsa é o `fetch`, que devolve o pacote já presente no disco e
 * um `DecompressionStream` do Node no lugar do do navegador.
 *
 * Serve para pegar regressão exatamente onde ela dói: nos caminhos do memfs,
 * nos argumentos do clang/wasm-ld e no shim WASI que executa o programa.
 *
 *   node playground/tools/verificar-navegador.mjs              # compila tudo
 *   node playground/tools/verificar-navegador.mjs --executar   # compila e roda
 *   node playground/tools/verificar-navegador.mjs --executar --exemplo "Olá com nome" --entrada "Maria\n"
 */

import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';

const AQUI = path.dirname(fileURLToPath(import.meta.url));
const RAIZ = path.resolve(AQUI, '..', '..');
const PACOTE = path.join(RAIZ, 'playground', 'publico', 'toolchain', 'wasi-cc.tar.gz');

if (!fs.existsSync(PACOTE)) {
  console.error(`falta ${path.relative(RAIZ, PACOTE)}; rode antes:\n`
    + '  node playground/tools/empacotar-toolchain.js');
  process.exit(2);
}

// ------------------------------------------------------------------ exemplos
// Lê exemplos.js como módulo (é o mesmo arquivo que a página usa).
async function lerExemplos() {
  const fonte = fs.readFileSync(path.join(RAIZ, 'playground', 'ativo', 'exemplos.js'), 'utf8');
  const temporario = path.join(AQUI, '.exemplos.tmp.mjs');
  fs.writeFileSync(temporario, fonte.replace(/^export const EXEMPLOS =/m, 'export default'));
  try {
    const { default: exemplos } = await import(`file://${temporario}`);
    return exemplos;
  } finally {
    fs.rmSync(temporario, { force: true });
  }
}

// ---------------------------------------------------------------- ambiente
// O único "navegador" que falta no Node: buscar o pacote. Devolvemos o arquivo
// do disco para o mesmo `fetch` que a página faz.
const bytesPacote = fs.readFileSync(PACOTE);
const fetchOriginal = globalThis.fetch;
globalThis.fetch = async (recurso, opcoes) => {
  const url = String(recurso && recurso.url ? recurso.url : recurso);
  if (url.includes('wasi-cc.tar.gz')) {
    const copia = bytesPacote.buffer.slice(
      bytesPacote.byteOffset, bytesPacote.byteOffset + bytesPacote.byteLength,
    );
    return {
      ok: true,
      status: 200,
      arrayBuffer: async () => copia,
      text: async () => '',
    };
  }
  return fetchOriginal(recurso, opcoes);
};

const { Ferramentas, prepararFonte } = await import(
  `file://${path.join(RAIZ, 'playground', 'ativo', 'ferramentas.js')}`
);
const { criarWasiPrograma } = await import(
  `file://${path.join(RAIZ, 'playground', 'ativo', 'armazenamento.js')}`
);

// ------------------------------------------------------------------- ajuda
const argumento = (nome) => {
  const i = process.argv.indexOf(nome);
  return i >= 0 ? process.argv[i + 1] : null;
};
const executar = process.argv.includes('--executar');
const filtro = argumento('--exemplo');
const entrada = (argumento('--entrada') || '').replace(/\\n/g, '\n');

// --------------------------------------------------------------- execução
/** Roda o wasm como a página faz: imports de armazenamento.js + stdin roteirizado. */
async function rodar(wasm) {
  const decodificador = new TextDecoder();
  let memoria = null;
  let restante = new TextEncoder().encode(entrada);
  let saida = '';
  let erro = '';

  const imports = criarWasiPrograma({
    obterMemoria: () => memoria,
    io: {
      escrever: (bytes, canal) => {
        const texto = decodificador.decode(bytes);
        if (canal === 'erro') erro += texto; else saida += texto;
      },
      ler: (n) => {
        const pedaco = restante.subarray(0, n);
        restante = restante.subarray(pedaco.length);
        return pedaco;
      },
    },
    argv: ['programa'],
  });

  const instancia = await WebAssembly.instantiate(await WebAssembly.compile(wasm), {
    wasi_unstable: imports,
    wasi_snapshot_preview1: imports,
  });
  memoria = instancia.exports.memory;

  let codigo = 0;
  try {
    instancia.exports._start();
  } catch (e) {
    if (typeof e.codigoSaida !== 'number') throw e;
    codigo = e.codigoSaida;
  }
  return { codigo, saida, erro };
}

const exemplos = await lerExemplos();
const nomes = filtro ? [filtro] : Object.keys(exemplos);
const ferramentas = new Ferramentas({
  aoProgredir: ({ etapa, mensagem }) => console.log(`  [${etapa}] ${mensagem}`),
  aoTraco: (linha) => console.log(`  · ${linha}`),
});

let falhas = 0;
for (const nome of nomes) {
  if (!(nome in exemplos)) {
    console.error(`exemplo desconhecido: ${nome}`);
    falhas += 1;
    continue;
  }
  process.stdout.write(`— ${nome} … `);
  const resultado = await ferramentas.compilar({ codigo: prepararFonte(exemplos[nome]) });
  if (!resultado.sucesso) {
    console.log('FALHOU\n');
    console.log(resultado.registro);
    falhas += 1;
    continue;
  }
  console.log(`ok (${(resultado.wasm.length / 1024).toFixed(1)} KB)`);
  if (!executar) continue;

  const x = await rodar(resultado.wasm);
  const linhas = x.saida.trimEnd().split('\n').slice(0, 6);
  console.log(`   código ${x.codigo}`);
  for (const linha of linhas) console.log(`   │ ${linha}`);
  if (x.erro.trim()) console.log(`   stderr: ${x.erro.trim()}`);
}

console.log(falhas
  ? `\n${falhas} exemplo(s) falharam.`
  : `\n${nomes.length} exemplo(s) ok (código do navegador, sem cópia).`);
process.exit(falhas ? 1 : 0);

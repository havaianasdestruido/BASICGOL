#!/usr/bin/env node
/*
 * verificar-navegador.mjs — roda **o código da página** (`ativo/*.js`) fora do
 * navegador, para provar que é ele quem compila e executa BASICGOL.
 *
 * Diferente de `verificar.js` (que reproduz o protocolo do memfs em CommonJS),
 * este script importa `ativo/ferramentas.js` e `ativo/armazenamento.js` como
 * módulos ES — os mesmos arquivos que a página carrega, sem cópia nenhuma. O
 * único "navegador" simulado é o `fetch`, que devolve o pacote do disco (e o
 * `DecompressionStream`, que no Node é o mesmo da web).
 *
 * É o teste que pega regressão onde dói: caminhos do memfs, argumentos do
 * clang/wasm-ld, montagem do disco e o shim WASI que executa o programa.
 *
 *   node playground/tools/verificar-navegador.mjs                # compila tudo
 *   node playground/tools/verificar-navegador.mjs --executar      # compila e roda
 *   node playground/tools/verificar-navegador.mjs --dourado       # confere a saída
 *   node playground/tools/verificar-navegador.mjs --executar --exemplo adivinha.c
 */

import fs from 'fs';
import path from 'path';
import { fileURLToPath } from 'url';

const AQUI = path.dirname(fileURLToPath(import.meta.url));
const RAIZ = path.resolve(AQUI, '..', '..');
const ATIVO = path.join(RAIZ, 'playground', 'ativo');
const PACOTE = path.join(RAIZ, 'playground', 'publico', 'toolchain', 'wasi-cc.tar.gz');

if (!fs.existsSync(PACOTE)) {
  console.error(`falta ${path.relative(RAIZ, PACOTE)}; rode antes:\n`
    + '  node playground/tools/empacotar-toolchain.js');
  process.exit(2);
}

const argumento = (nome) => {
  const i = process.argv.indexOf(nome);
  return i >= 0 ? process.argv[i + 1] : null;
};
const executar = process.argv.includes('--executar');
const dourado = process.argv.includes('--dourado');
const filtro = argumento('--exemplo');
const entrada = (argumento('--entrada') || '').replace(/\\n/g, '\n');

// ---------------------------------------------------------------- ambiente
// A página busca o pacote com `fetch(new URL(...))`. Aqui respondemos com o
// arquivo do disco; todo o resto (descompressão, memfs, clang) é o código real.
const bytesPacote = fs.readFileSync(PACOTE);
function descompactar(bytes, formato) {
  const fluxo = new Blob([bytes]).stream().pipeThrough(new DecompressionStream(formato));
  return new Response(fluxo).arrayBuffer().then((b) => new Uint8Array(b));
}

const fetchOriginal = globalThis.fetch;
globalThis.fetch = async (recurso, opcoes) => {
  const url = String(recurso && recurso.url ? recurso.url : recurso);
  if (url.includes('wasi-cc.tar.gz')) {
    const copia = bytesPacote.buffer.slice(
      bytesPacote.byteOffset, bytesPacote.byteOffset + bytesPacote.byteLength,
    );
    return { ok: true, status: 200, arrayBuffer: async () => copia, text: async () => '' };
  }
  if (url.includes('icones.json')) {
    const texto = fs.readFileSync(path.join(RAIZ, 'playground', 'vendor', 'fa', 'icones.json'), 'utf8');
    return { ok: true, status: 200, json: async () => JSON.parse(texto), text: async () => texto };
  }
  return fetchOriginal(recurso, opcoes);
};

// `performance` existe no Node; `DecompressionStream` também. Nada mais falta.

const { Ferramentas, prepararFonte } = await import(`file://${path.join(ATIVO, 'ferramentas.js')}`);
const { criarWasiPrograma } = await import(`file://${path.join(ATIVO, 'armazenamento.js')}`);
const { EXEMPLOS } = await import(`file://${path.join(ATIVO, 'exemplos.js')}`);

// --------------------------------------------------------------- execução
/** Roda o wasm como a página faz: imports de armazenamento.js + roteiro de entrada. */
async function rodar(wasm, textoEntrada) {
  const decodificador = new TextDecoder();
  let memoria = null;
  let restante = new TextEncoder().encode(textoEntrada || '');
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

const lista = EXEMPLOS.filter((e) => !filtro
  || e.arquivo === filtro || e.arquivo === `${filtro}.c` || e.titulo === filtro);
if (!lista.length) {
  console.error(`nenhum exemplo corresponde a "${filtro}"`);
  process.exit(2);
}

const ferramentas = new Ferramentas({
  aoProgredir: ({ etapa, mensagem }) => console.log(`  [${etapa}] ${mensagem}`),
  aoTraco: (linha) => console.log(`  · ${linha}`),
});

let falhas = 0;
for (const exemplo of lista) {
  process.stdout.write(`— ${exemplo.arquivo} … `);
  const preparado = prepararFonte(exemplo.codigo);
  const resultado = await ferramentas.compilar({
    codigo: preparado.fonte,
    deslocamentoLinhas: preparado.deslocamento,
  });
  if (!resultado.sucesso) {
    console.log(`FALHOU\n\n${resultado.registro}`);
    falhas += 1;
    continue;
  }
  console.log(`ok (${(resultado.wasm.length / 1024).toFixed(1)} KB)`);
  if (!executar && !dourado) continue;

  const usado = entrada || exemplo.entrada || '';
  const x = await rodar(resultado.wasm, usado);

  if (dourado) {
    if (!exemplo.entrada && !fs.existsSync(path.join(RAIZ, 'docs', 'exemplos',
      exemplo.arquivo.replace(/\.c$/, '.esperado.txt')))) {
      console.log('      (sem .esperado.txt; nada a conferir)');
      continue;
    }
    const esperado = fs.readFileSync(path.join(RAIZ, 'docs', 'exemplos',
      exemplo.arquivo.replace(/\.c$/, '.esperado.txt')), 'utf8').replace(/\r\n/g, '\n');
    if (x.saida === esperado && x.codigo === 0) {
      console.log('      igual ao esperado');
      continue;
    }
    falhas += 1;
    console.log(`      DIFERENTE do esperado (código ${x.codigo})`);
    const esperadas = esperado.split('\n');
    const obtidas = x.saida.split('\n');
    for (let i = 0; i < Math.max(esperadas.length, obtidas.length); i += 1) {
      if (esperadas[i] === obtidas[i]) continue;
      console.log(`        linha ${i + 1}: esperado ${JSON.stringify(esperadas[i])}, `
        + `obtido ${JSON.stringify(obtidas[i])}`);
      if (i > 12) break;
    }
  } else {
    const linhas = x.saida.trimEnd().split('\n').slice(0, 6);
    console.log(`      código ${x.codigo}`);
    for (const linha of linhas) console.log(`      │ ${linha}`);
    if (x.erro.trim()) console.log(`      stderr: ${x.erro.trim()}`);
  }
}

if (dourado) {
  console.log(falhas
    ? `\n${falhas} exemplo(s) fora do esperado.`
    : `\n${lista.length} exemplo(s) ok e iguais a docs/exemplos/*.esperado.txt.`);
} else {
  console.log(falhas
    ? `\n${falhas} exemplo(s) falharam.`
    : `\n${lista.length} exemplo(s) ok — código do navegador, sem cópia.`);
}
process.exit(falhas ? 1 : 0);

#!/usr/bin/env node
/*
 * empacotar-toolchain.js — monta playground/publico/toolchain/wasi-cc.tar.gz,
 * o conjunto de ferramentas que o playground baixa e roda **no navegador**.
 *
 * O que entra no pacote (e por quê):
 *
 *   clang, lld, memfs  clang e wasm-ld do LLVM compilados para WebAssembly pelo
 *                      projeto binji/wasm-clang (Apache-2.0 / LLVM). São eles
 *                      que compilam o seu programa e a biblioteca BASICGOL
 *                      dentro da página; o `memfs` é o "disco" deles.
 *   sysroot.tar        wasi-libc + libclang_rt (builtins) + cabeçalhos, do
 *                      mesmo projeto. É a libc com que o programa final liga.
 *   include/           o cabeçalho BASICGOL do repositório.
 *   src/               a biblioteca padrão compilável (src/biblioteca/*.c) e o
 *                      `shim.c` do playground (stdio sem buffer, mkstemp).
 *   basicgol_playground.h
 *                      declarações que a libc do playground não traz.
 *
 * O arquivo gerado é artefato de build: não é versionado (ver .gitignore).
 * Rode este script uma vez, com rede, antes de servir o playground:
 *
 *   node playground/tools/empacotar-toolchain.js
 *
 * A origem é `playground/.toolchain-build/wasm-clang-master/`, baixada por
 * `tools/baixar-toolchain.sh`.
 */
'use strict';

const fs = require('fs');
const path = require('path');
const zlib = require('zlib');

const RAIZ = path.resolve(__dirname, '..', '..');
const ORIGEM = path.join(RAIZ, 'playground', '.toolchain-build', 'wasm-clang-master');
const DESTINO = path.join(RAIZ, 'playground', 'publico', 'toolchain');
const ARQUIVO = path.join(DESTINO, 'wasi-cc.tar.gz');

if (!fs.existsSync(path.join(ORIGEM, 'clang'))) {
  console.error(`falta ${path.relative(RAIZ, ORIGEM)}/clang — rode antes:\n`
    + '  playground/tools/baixar-toolchain.sh');
  process.exit(2);
}

// --------------------------------------------------------------------- tar
/** Monta um cabeçalho ustar (512 bytes) para `nome`. */
function cabecalho(nome, tamanho, modo, mtime) {
  const h = Buffer.alloc(512, 0);
  const escrever = (texto, posicao, largura) => {
    h.write(texto, posicao, largura, 'utf8');
  };
  const octal = (valor, largura) => `${valor.toString(8).padStart(largura - 1, '0')}\0`;

  if (Buffer.byteLength(nome) > 100) {
    // ustar: prefixo (155 bytes) + nome (100 bytes)
    const partes = nome.split('/');
    let corte = partes.length;
    let sufixo = partes[partes.length - 1];
    while (corte > 1 && Buffer.byteLength(sufixo) <= 100) {
      corte -= 1;
      sufixo = `${partes[corte]}/${sufixo}`;
    }
    const prefixo = partes.slice(0, corte).join('/');
    if (Buffer.byteLength(prefixo) > 155 || Buffer.byteLength(sufixo) > 100) {
      throw new Error(`caminho grande demais para o tar ustar: ${nome}`);
    }
    escrever(sufixo, 0, 100);
    escrever(prefixo, 345, 155);
  } else {
    escrever(nome, 0, 100);
  }
  escrever(octal(modo, 8), 100, 8); // modo
  escrever(octal(0, 8), 108, 8); // uid
  escrever(octal(0, 8), 116, 8); // gid
  escrever(octal(tamanho, 12), 124, 12); // tamanho
  escrever(octal(Math.floor(mtime / 1000), 12), 136, 12); // mtime
  escrever('        ', 148, 8); // checksum (espaços no cálculo)
  h[156] = modo === 0o755 && tamanho === 0 ? 0x35 : 0x30; // tipo: '5' dir, '0' arquivo
  escrever('ustar\0', 257, 6);
  escrever('00', 263, 2);
  let soma = 0;
  for (let i = 0; i < 512; i += 1) soma += h[i];
  escrever(`${soma.toString(8).padStart(6, '0')}\0 `, 148, 8);
  return h;
}

/** Devolve os blocos (Buffers) de uma entrada de arquivo. */
function blocosDeArquivo(nome, bytes, mtime) {
  const partes = [cabecalho(nome, bytes.length, 0o644, mtime), bytes];
  const sobra = bytes.length % 512;
  if (sobra) partes.push(Buffer.alloc(512 - sobra, 0));
  return partes;
}

// ------------------------------------------------------------------ origem
const entradas = []; // { nome, origem }
const adicionar = (nome, origem) => entradas.push({ nome, origem });

adicionar('clang', path.join(ORIGEM, 'clang'));
adicionar('lld', path.join(ORIGEM, 'lld'));
adicionar('memfs', path.join(ORIGEM, 'memfs'));
adicionar('sysroot.tar', path.join(ORIGEM, 'sysroot.tar'));

/** Copia uma árvore do repositório para dentro do pacote, sob `prefixo`. */
function adicionarArvore(prefixo, origem) {
  for (const entrada of fs.readdirSync(origem, { withFileTypes: true }).sort((a, b) => a.name.localeCompare(b.name))) {
    const de = path.join(origem, entrada.name);
    const para = `${prefixo}/${entrada.name}`;
    if (entrada.isDirectory()) adicionarArvore(para, de);
    else adicionar(para, de);
  }
}

adicionarArvore('include', path.join(RAIZ, 'include'));
adicionarArvore('src', path.join(RAIZ, 'src'));
adicionar('src/shim.c', path.join(__dirname, 'shim.c'));

// Compatibilidade da libc do playground (injetado com `-include`).
adicionar('basicgol_playground.h', path.join(__dirname, 'basicgol_playground.h'));

// Depurador opcional da página: só é injetado (também com `-include`, e só no
// arquivo do usuário) quando o depurador está ligado — ele vem desligado.
adicionar('basicgol_depurador.h', path.join(__dirname, 'basicgol_depurador.h'));

// O `filesystem.c` da biblioteca chama funções de sistema de arquivos que não
// existem na wasi-libc de 2019 usada aqui (mkstemp). O shim declara e responde
// ENOSYS; compilar o arquivo é melhor do que sumir com ele: o resto do módulo
// (que não depende de disco) continua disponível.

// ------------------------------------------------------------------- versão
const versao = [
  `BASICGOL playground toolchain`,
  `clang/lld: binji/wasm-clang (LLVM 8.0.1, wasm32-unknown-wasi)`,
  `gerado: ${new Date().toISOString()}`,
  `arquivos do repositório: include/, src/biblioteca/, src/shim.c, basicgol_playground.h,`
  + ` basicgol_depurador.h`,
  '',
].join('\n');
const temporarioVersao = path.join(__dirname, '.VERSAO.txt');
fs.writeFileSync(temporarioVersao, versao);
adicionar('VERSAO.txt', temporarioVersao);

// --------------------------------------------------------------------- tar
const agora = Date.now();
const blocos = [];
const vistos = new Set();

// Diretórios antes dos filhos (o memfs exige o pai existente), sem repetição.
const diretorios = new Set();
for (const { nome } of entradas) {
  const partes = nome.split('/');
  partes.pop();
  const parcial = [];
  for (const parte of partes) {
    parcial.push(parte);
    const caminho = parcial.join('/');
    if (!diretorios.has(caminho)) diretorios.add(caminho);
  }
}
for (const diretorio of [...diretorios].sort()) {
  blocos.push(cabecalho(`${diretorio}/`, 0, 0o755, agora));
}

for (const { nome, origem } of entradas) {
  if (vistos.has(nome)) {
    throw new Error(`entrada duplicada no pacote: ${nome}`);
  }
  vistos.add(nome);
  const bytes = fs.readFileSync(origem);
  blocos.push(...blocosDeArquivo(nome, bytes, fs.statSync(origem).mtimeMs));
  process.stdout.write(`  ${nome} (${(bytes.length / 1024).toFixed(0)} KB)\n`);
}
blocos.push(Buffer.alloc(1024, 0)); // fim do arquivo: dois blocos nulos

const tar = Buffer.concat(blocos);
fs.rmSync(temporarioVersao, { force: true });
fs.mkdirSync(DESTINO, { recursive: true });
fs.writeFileSync(ARQUIVO, zlib.gzipSync(tar, { level: 9 }));

console.log(`\npronto: ${path.relative(RAIZ, ARQUIVO)} `
  + `(${(fs.statSync(ARQUIVO).size / 1024 / 1024).toFixed(1)} MB, ${entradas.length} arquivos)`);

#!/usr/bin/env node
/*
 * verificar-depurador.mjs — prova o depurador opcional da página.
 *
 * Ele importa os mesmos módulos que a aba carrega (`ativo/depurador.js`,
 * `ativo/ferramentas.js`, `ativo/armazenamento.js`) e verifica as três coisas
 * que o depurador promete:
 *
 *   1. **não muda o programa**: cada exemplo de `docs/exemplos/` é compilado
 *      instrumentado e a saída tem de ser idêntica ao `.esperado.txt` — os
 *      quadros do depurador saem pelo descritor 3, nunca pelo stdout;
 *   2. **vê o que está acontecendo**: as linhas percorridas e os valores das
 *      variáveis chegam à página (e as linhas batem com o editor);
 *   3. **para onde foi pedido**: a máquina de pontos de parada/passo/parar é
 *      exercitada com quadros sintéticos (pausa de verdade exige JSPI, que só
 *      o navegador tem).
 *
 *   node playground/tools/verificar-depurador.mjs
 *   node playground/tools/verificar-depurador.mjs --exemplo tabuada.c
 *   node playground/tools/verificar-depurador.mjs --quadros   # mostra os quadros
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
const filtro = argumento('--exemplo');
const mostrarQuadros = process.argv.includes('--quadros');

// ---------------------------------------------------------------- ambiente
const bytesPacote = fs.readFileSync(PACOTE);
const fetchOriginal = globalThis.fetch;
globalThis.fetch = async (recurso, opcoes) => {
  const url = String(recurso && recurso.url ? recurso.url : recurso);
  if (url.includes('wasi-cc.tar.gz')) {
    const copia = bytesPacote.buffer.slice(
      bytesPacote.byteOffset, bytesPacote.byteOffset + bytesPacote.byteLength,
    );
    return { ok: true, status: 200, arrayBuffer: async () => copia };
  }
  return fetchOriginal(recurso, opcoes);
};

const { Ferramentas, prepararFonte } = await import(`file://${path.join(ATIVO, 'ferramentas.js')}`);
const { criarWasiPrograma } = await import(`file://${path.join(ATIVO, 'armazenamento.js')}`);
const { EXEMPLOS } = await import(`file://${path.join(ATIVO, 'exemplos.js')}`);
const { instrumentar, Depurador, COMANDOS, SAIDA_PARADO } = await import(`file://${path.join(ATIVO, 'depurador.js')}`);

const falhas = [];
const conferir = (ok, descricao, detalhe = '') => {
  console.log(`${ok ? '  ok  ' : ' FALHA'}  ${descricao}${detalhe ? ` — ${detalhe}` : ''}`);
  if (!ok) falhas.push(descricao);
};

/* ===================================================================== */
/* 1. a máquina de parada, com quadros sintéticos                        */
/* ===================================================================== */

/** Monta o texto de um quadro como o programa manda pelo descritor 3. */
const quadro = (linha, variaveis = []) => new TextEncoder().encode(
  `l ${linha}\n${variaveis.map((v) => `v ${v[0]} ${v[1]} ${v[2]}\n`).join('')}e\n`,
);
const comando = (resposta) => new TextDecoder().decode(resposta);

async function verificarMaquina() {
  console.log('— máquina de pontos de parada (quadros sintéticos)');

  // (a) desligado: nada é instrumentado e nada para.
  const d = new Depurador();
  conferir(d.ativo === false, 'o depurador começa desligado');

  // (b) sem JSPI: modo traço, nunca pausa, mas registra tudo.
  d.definirAtivo(true);
  d.iniciar({ deslocamento: 0, podePausar: false });
  const canalTraco = d.canal();
  canalTraco.escrever(quadro(7, [['i', 'inteiro', '3']]));
  const respostaTraco = canalTraco.ler();
  conferir(comando(respostaTraco) === COMANDOS.CONTINUAR,
    'sem JSPI o programa nunca congela (modo traço)');
  conferir(d.historico.length === 1 && d.historico[0].linha === 7,
    'o traço registra a linha percorrida');

  // (c) com JSPI: para na primeira linha, anda de passo em passo e segue
  //     até o próximo ponto de parada.
  d.iniciar({ deslocamento: 3, podePausar: true });
  const canal = d.canal();
  canal.escrever(quadro(10, [['soma', 'inteiro', '42'], ['nome', 'texto', '"Ana"']]));
  const pendente = canal.ler();
  conferir(typeof pendente.then === 'function',
    'com JSPI a primeira linha devolve uma Promise (o programa espera)');
  conferir(d.estaParado() && d.linha === 7,
    'a linha do quadro vira a linha do editor', `linha ${d.linha}`);
  conferir(d.variaveis.length === 2 && d.variaveis[0].valor === '42'
    && d.variaveis[1].valor === '"Ana"', 'as variáveis do quadro chegam à página');

  d.passo();
  conferir(comando(await pendente) === COMANDOS.PASSO, 'o botão Passo responde ao programa');

  canal.escrever(quadro(11));
  conferir(typeof canal.ler().then === 'function', 'depois de um passo, para na linha seguinte');
  d.continuar();

  // Sem ponto de parada, continuar não para mais.
  canal.escrever(quadro(12));
  conferir(comando(canal.ler()) === COMANDOS.CONTINUAR, 'Continuar segue sem parar');

  // Com um ponto de parada na linha 20 do editor (= 23 do arquivo).
  d.alternarPonto(20);
  canal.escrever(quadro(23));
  conferir(typeof canal.ler().then === 'function', 'o programa para no ponto de parada');
  conferir(d.linha === 20, 'o ponto de parada é contado em linha do editor');

  // Parar encerra o programa.
  d.parar();
  canal.escrever(quadro(24));
  conferir(comando(canal.ler()) === COMANDOS.PARAR, 'o botão Parar encerra o programa');

  // Limite de eventos: o depurador se desliga e o programa segue.
  const limitado = new Depurador({ limite: 3 });
  limitado.definirAtivo(true);
  limitado.iniciar({ podePausar: false });
  const canalLimite = limitado.canal();
  let ultima = '';
  for (let i = 0; i < 5; i += 1) {
    canalLimite.escrever(quadro(i + 1));
    ultima = comando(canalLimite.ler());
  }
  conferir(ultima === COMANDOS.DESLIGAR && limitado.aviso !== '',
    'passado o limite de eventos, o depurador se desliga sozinho');
}

/* ===================================================================== */
/* 2. os exemplos, compilados com o depurador ligado                     */
/* ===================================================================== */

/** Roda o wasm com o canal do depurador no descritor 3. */
async function rodar(wasm, textoEntrada, depurador) {
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
    depuracao: depurador ? depurador.canal() : null,
  });

  const instancia = await WebAssembly.instantiate(
    await WebAssembly.compile(wasm),
    { wasi_unstable: imports, wasi_snapshot_preview1: imports },
  );
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

async function verificarExemplos() {
  const lista = EXEMPLOS.filter((e) => !filtro
    || e.arquivo === filtro || e.arquivo === `${filtro}.c` || e.titulo === filtro);
  if (!lista.length) {
    console.error(`nenhum exemplo corresponde a "${filtro}"`);
    process.exit(2);
  }

  const ferramentas = new Ferramentas({ aoProgredir: () => {}, aoTraco: null });
  let comVariaveis = 0;

  for (const exemplo of lista) {
    const esperadoEm = path.join(RAIZ, 'docs', 'exemplos',
      exemplo.arquivo.replace(/\.c$/, '.esperado.txt'));
    if (!fs.existsSync(esperadoEm)) continue;

    const preparado = prepararFonte(exemplo.codigo);
    const instrumentado = instrumentar(preparado.fonte);
    const linhasOriginais = preparado.fonte.split('\n').length;
    const linhasInstrumentadas = instrumentado.fonte.split('\n').length;

    const depurador = new Depurador({ limite: 1000000 });
    depurador.definirAtivo(true);
    depurador.iniciar({ deslocamento: preparado.deslocamento, podePausar: false });

    const resultado = await ferramentas.compilar({
      codigo: instrumentado.fonte,
      deslocamentoLinhas: preparado.deslocamento,
      depurar: true,
    });
    if (!resultado.sucesso) {
      conferir(false, `${exemplo.arquivo}: compila instrumentado`,
        resultado.registro.split('\n').filter((l) => /error/.test(l)).slice(0, 3).join(' | '));
      continue;
    }

    const execucao = await rodar(resultado.wasm, exemplo.entrada || '', depurador);
    const esperado = fs.readFileSync(esperadoEm, 'utf8').replace(/\r\n/g, '\n');
    const avisos = resultado.registro.split('\n').filter((l) => /warning:/.test(l));

    const iguais = execucao.saida === esperado && execucao.codigo === 0;
    const linhasOk = linhasInstrumentadas === linhasOriginais;
    const viuLinhas = depurador.eventos > 0;
    const viuVariaveis = depurador.historico.some((h) => h.variaveis.length > 0);
    if (viuVariaveis) comVariaveis += 1;

    const detalhe = `${depurador.eventos} linhas, `
      + `${instrumentado.linhas.length} pontos possíveis`
      + (avisos.length ? `, ${avisos.length} aviso(s)` : '');
    conferir(iguais && linhasOk && viuLinhas && !avisos.length,
      `${exemplo.arquivo}: saída idêntica ao dourado com o depurador ligado`, detalhe);
    if (!iguais) {
      const esperadas = esperado.split('\n');
      const obtidas = execucao.saida.split('\n');
      for (let i = 0; i < Math.max(esperadas.length, obtidas.length); i += 1) {
        if (esperadas[i] === obtidas[i]) continue;
        console.log(`        linha ${i + 1}: esperado ${JSON.stringify(esperadas[i])}, `
          + `obtido ${JSON.stringify(obtidas[i])}`);
        break;
      }
    }
    if (avisos.length) console.log(`        ${avisos.slice(0, 3).join('\n        ')}`);
    if (mostrarQuadros) {
      for (const h of depurador.historico.slice(0, 8)) {
        console.log(`        linha ${h.linha}: `
          + h.variaveis.map((v) => `${v.nome}=${v.valor}`).join(', '));
      }
    }
  }

  conferir(comVariaveis > 0, 'os valores das variáveis chegam em pelo menos um exemplo',
    `${comVariaveis} exemplo(s)`);
}

/* ===================================================================== */
/* 3. o programa de boas-vindas, com ponto de parada e Parar             */
/* ===================================================================== */

async function verificarParada() {
  console.log('— o botão Parar encerra o programa de verdade');
  const programa = [
    '#include <basicgol.h>',
    '',
    'INICIO',
    '    INTEIRO i = 0;',
    '    PARA (i, 1, 1000) {',
    '        ESCREVA_FMT("%d\\n", i);',
    '    }',
    'FIM',
    '',
  ].join('\n');

  const ferramentas = new Ferramentas({ aoProgredir: () => {}, aoTraco: null });
  const preparado = prepararFonte(programa);
  const instrumentado = instrumentar(preparado.fonte);
  const resultado = await ferramentas.compilar({
    codigo: instrumentado.fonte,
    deslocamentoLinhas: preparado.deslocamento,
    depurar: true,
  });
  if (!resultado.sucesso) {
    conferir(false, 'compila o laço instrumentado', resultado.registro.slice(-300));
    return;
  }

  // Sem JSPI não dá para congelar o programa, mas o pedido de parada vale a
  // partir do próximo quadro: é exatamente o que o botão Parar faz.
  const depurador = new Depurador();
  depurador.definirAtivo(true);
  depurador.iniciar({ deslocamento: preparado.deslocamento, podePausar: false });
  const canal = depurador.canal();
  const original = canal.escrever;
  let quadros = 0;
  canal.escrever = (bytes) => {
    original(bytes);
    quadros += 1;
    if (quadros === 5) depurador.parar();
  };
  const execucao = await rodar(resultado.wasm, '', { canal: () => canal });
  conferir(execucao.codigo === SAIDA_PARADO,
    'o programa termina com o código do depurador', `código ${execucao.codigo}`);
  conferir(execucao.saida.split('\n').filter(Boolean).length < 10,
    'o laço de 1000 voltas foi interrompido', `${execucao.saida.split('\n').filter(Boolean).length} linhas impressas`);
}

/* ===================================================================== */

await verificarMaquina();
console.log('— exemplos de docs/exemplos/ com o depurador ligado');
await verificarExemplos();
await verificarParada();

if (falhas.length) {
  console.error(`\n${falhas.length} verificação(ões) falharam.`);
  process.exit(1);
}
console.log('\ndepurador verificado: não muda a saída, mostra as variáveis e para onde foi pedido.');

#!/usr/bin/env node
/*
 * verificar-pagina.mjs — o teste de fumaça da página de verdade.
 *
 * `verificar-navegador.mjs` importa o código da página e prova que ele compila
 * e executa; este aqui abre `ativo/index.html` num Chromium headless, aperta o
 * Run e confere o que só a página pode provar: que o terminal mostra a saída,
 * que a entrada interativa (JSPI) espera o Enter, que o seletor de exemplos
 * carrega o programa com a entrada de teste e que não há erro de console.
 *
 *   node playground/tools/verificar-pagina.mjs
 *   node playground/tools/verificar-pagina.mjs --chromium /caminho/do/chromium
 *   node playground/tools/verificar-pagina.mjs --manter   # não fecha o servidor
 *
 * O Chromium é achado, nesta ordem: `--chromium`, `$CHROMIUM`,
 * `$BASICGOL_CHROMIUM`, `chromium`/`google-chrome` no PATH. Se não houver
 * nenhum, o script explica e sugere `--sem-navegador` (que roda a versão sem
 * DOM, `verificar-navegador.mjs --dourado`).
 *
 * A página é servida por um servidor estático mínimo embutido (nada de `python
 * -m http.server`): os módulos ES precisam de http, não de file://.
 */

import fs from 'fs';
import http from 'http';
import path from 'path';
import { spawn } from 'child_process';
import { fileURLToPath } from 'url';

const AQUI = path.dirname(fileURLToPath(import.meta.url));
const RAIZ = path.resolve(AQUI, '..', '..');
const PAGINA = path.join(RAIZ, 'playground', 'ativo', 'index.html');

const argumento = (nome) => {
  const i = process.argv.indexOf(nome);
  return i >= 0 ? process.argv[i + 1] : null;
};

const TIPOS = {
  '.html': 'text/html; charset=utf-8',
  '.js': 'text/javascript; charset=utf-8',
  '.mjs': 'text/javascript; charset=utf-8',
  '.css': 'text/css; charset=utf-8',
  '.json': 'application/json; charset=utf-8',
  '.svg': 'image/svg+xml',
  '.woff2': 'font/woff2',
  '.png': 'image/png',
  '.tar.gz': 'application/gzip',
};

/* ---------------------------------------------------------------- servidor */

/** Servidor estático só para esta verificação (mesma origem para tudo). */
function servir() {
  const servidor = http.createServer((req, res) => {
    const caminho = path.join(RAIZ, decodeURIComponent(new URL(req.url, 'http://x').pathname));
    if (!caminho.startsWith(RAIZ) || !fs.existsSync(caminho) || fs.statSync(caminho).isDirectory()) {
      res.writeHead(404).end('não encontrado');
      return;
    }
    const extensao = caminho.endsWith('.tar.gz') ? '.tar.gz' : path.extname(caminho);
    res.writeHead(200, {
      'Content-Type': TIPOS[extensao] || 'application/octet-stream',
      'Cache-Control': 'no-store',
    });
    fs.createReadStream(caminho).pipe(res);
  });
  return new Promise((resolver) => {
    servidor.listen(0, '127.0.0.1', () => resolver(servidor));
  });
}

/* ----------------------------------------------------------------- chrome */

function acharChromium() {
  const candidatos = [
    argumento('--chromium'),
    process.env.BASICGOL_CHROMIUM,
    process.env.CHROMIUM,
    '/usr/bin/chromium',
    '/usr/bin/chromium-browser',
    '/usr/bin/google-chrome',
    '/tmp/chrom/chromium',
  ].filter(Boolean);
  for (const candidato of candidatos) {
    if (fs.existsSync(candidato)) return { caminho: candidato, extra: [] };
  }
  return null;
}

/** Fala o protocolo DevTools por WebSocket (sem dependência nenhuma). */
async function abrirChrome(executavel) {
  const porta = 9300 + Math.floor(Math.random() * 400);
  const processo = spawn(executavel, [
    '--headless=new', '--no-sandbox', '--disable-gpu', '--disable-dev-shm-usage',
    `--remote-debugging-port=${porta}`, '--window-size=1400,900', 'about:blank',
  ], {
    stdio: 'ignore',
    // O Chromium do @sparticuz carrega as .so de /tmp/chrom/lib/lib.
    env: { ...process.env, LD_LIBRARY_PATH: process.env.LD_LIBRARY_PATH || '' },
  });
  for (let i = 0; i < 80; i += 1) {
    try {
      const resposta = await fetch(`http://127.0.0.1:${porta}/json/version`);
      if (resposta.ok) break;
    } catch (e) { /* ainda subindo */ }
    await new Promise((r) => setTimeout(r, 250));
  }
  return { processo, porta };
}

async function conectar(porta) {
  const alvo = await (await fetch(`http://127.0.0.1:${porta}/json/new?about:blank`, { method: 'PUT' })).json();
  const ws = new WebSocket(alvo.webSocketDebuggerUrl);
  let id = 0;
  const pendentes = new Map();
  const eventos = [];
  ws.onmessage = (m) => {
    const msg = JSON.parse(m.data);
    if (msg.id && pendentes.has(msg.id)) {
      const p = pendentes.get(msg.id);
      pendentes.delete(msg.id);
      if (msg.error) p.rejeitar(new Error(JSON.stringify(msg.error)));
      else p.resolver(msg.result);
    } else if (msg.method) {
      eventos.push(msg);
    }
  };
  await new Promise((r) => { ws.onopen = r; });
  const enviar = (method, params = {}) => new Promise((resolver, rejeitar) => {
    const i = ++id;
    pendentes.set(i, { resolver, rejeitar });
    ws.send(JSON.stringify({ id: i, method, params }));
  });
  return { ws, enviar, eventos };
}

/* ------------------------------------------------------------------ testes */

const dormir = (ms) => new Promise((r) => setTimeout(r, ms));

async function principal() {
  if (!fs.existsSync(path.join(RAIZ, 'playground', 'publico', 'toolchain', 'wasi-cc.tar.gz'))) {
    console.error('falta playground/publico/toolchain/wasi-cc.tar.gz; rode antes:\n'
      + '  node playground/tools/empacotar-toolchain.js');
    process.exit(2);
  }

  const chromium = acharChromium();
  if (!chromium) {
    console.error('nenhum Chromium encontrado. Passe --chromium /caminho/do/chromium\n'
      + 'ou defina BASICGOL_CHROMIUM. Para a verificação sem navegador:\n'
      + '  node playground/tools/verificar-navegador.mjs --dourado');
    process.exit(2);
  }

  const servidor = await servir();
  const url = `http://127.0.0.1:${servidor.address().port}/playground/ativo/index.html`;
  const { processo, porta } = await abrirChrome(chromium.caminho);
  const { ws, enviar, eventos } = await conectar(porta);
  const falhas = [];
  const conferir = (ok, descricao, detalhe = '') => {
    console.log(`${ok ? '  ok  ' : ' FALHA'}  ${descricao}${detalhe ? ` — ${detalhe}` : ''}`);
    if (!ok) falhas.push(descricao);
  };

  const avaliar = async (expressao) => {
    const r = await enviar('Runtime.evaluate', {
      expression: expressao, awaitPromise: true, returnByValue: true,
    });
    if (r.exceptionDetails) {
      throw new Error((r.exceptionDetails.exception || {}).description || r.exceptionDetails.text);
    }
    return r.result.value;
  };
  const tela = () => avaliar(`(() => {
    const b = window.__basicgol.terminal.xterm.buffer.active;
    const linhas = [];
    for (let i = 0; i < b.length; i += 1) {
      const l = b.getLine(i);
      if (l) linhas.push(l.translateToString(true));
    }
    return linhas.join('\\n').replace(/\\n+$/, '');
  })()`);

  try {
    await enviar('Runtime.enable');
    await enviar('Log.enable');
    await enviar('Page.enable');
    await enviar('Page.navigate', { url });

    // A toolchain leva alguns segundos para baixar e desempacotar.
    let pronta = false;
    for (let i = 0; i < 60 && !pronta; i += 1) {
      await dormir(500);
      pronta = await avaliar(`!!(window.__basicgol && window.__basicgol.ferramentas
        && document.querySelector('#toolchain-status').textContent.includes('prontas'))`);
    }
    conferir(pronta, 'a página carregou e a toolchain ficou pronta');

    // 1) Executar o programa inicial (boas-vindas) e ver a saída no terminal.
    await avaliar(`document.querySelector('#run-btn').click(), 1`);
    let saida = '';
    for (let i = 0; i < 40; i += 1) {
      await dormir(500);
      saida = await tela();
      // O programa de boas-vindas termina pedindo um nome: ou ele para na
      // leitura, ou (sem JSPI) lê EOF e termina sozinho.
      if (/iteracao 3/.test(saida) && (/Digite o seu nome/.test(saida))) break;
    }
    conferir(/iteracao 3/.test(saida), 'o programa compilou no navegador e rodou');
    conferir(/compilado em [\d.]+\s*s/.test(saida), 'o terminal mostra os tempos de compilação');

    // 2) Entrada interativa: o programa para no LEIA_TEXTO e retoma com o Enter.
    const esperaEntrada = await avaliar(`window.__basicgol.terminal.emEdicao`);
    if (esperaEntrada) {
      await avaliar(`window.__basicgol.terminal.xterm.focus(), 1`);
      await enviar('Input.insertText', { text: 'Maria' });
      await dormir(200);
      await enviar('Input.dispatchKeyEvent', { type: 'rawKeyDown', windowsVirtualKeyCode: 13, key: 'Enter', code: 'Enter' });
      await enviar('Input.dispatchKeyEvent', { type: 'char', text: '\r' });
      await enviar('Input.dispatchKeyEvent', { type: 'keyUp', windowsVirtualKeyCode: 13, key: 'Enter', code: 'Enter' });
      for (let i = 0; i < 20; i += 1) {
        await dormir(300);
        saida = await tela();
        if (/Ola, Maria!/.test(saida)) break;
      }
      conferir(/Ola, Maria!/.test(saida), 'a entrada digitada chega ao programa (JSPI)');
    } else {
      console.log('  --    entrada interativa indisponível neste Chromium (sem JSPI), teste pulado');
    }

    // 3) Seletor de exemplos: carrega o programa e a entrada de teste dourada.
    await avaliar(`(() => {
      const s = document.querySelector('#examples-select');
      s.value = 'tabuada.c';
      s.dispatchEvent(new Event('change'));
      return 1;
    })()`);
    await dormir(400);
    conferir(/tabuada/.test(await avaliar(`window.__basicgol.editor.getValue()`)),
      'o seletor de exemplos carrega o código');
    await avaliar(`document.querySelector('#run-btn').click(), 1`);
    for (let i = 0; i < 40; i += 1) {
      await dormir(400);
      saida = await tela();
      if (/\[o programa terminou/.test(saida)) break;
    }
    const dourado = fs.readFileSync(path.join(RAIZ, 'docs', 'exemplos', 'tabuada.esperado.txt'), 'utf8')
      .replace(/\n+$/, '');
    const corpo = saida.split(/\r?\n/).filter((l) => l !== '');
    const corpoDourado = dourado.split(/\r?\n/).filter((l) => l !== '');
    conferir(corpoDourado.every((l) => corpo.includes(l)),
      'a saída do exemplo no terminal bate com docs/exemplos/tabuada.esperado.txt');

    // 4) O depurador: desligado por padrão, e ligado ele para de verdade.
    conferir(await avaliar(`!document.querySelector('#debug-toggle').checked
      && document.querySelector('#debug-pane').classList.contains('oculto')
      && !document.body.classList.contains('depurando')
      && !window.__basicgol.depurador.ativo`),
    'o depurador vem desligado (painel escondido, Run continua Run)');

    const programa = [
      '#include <basicgol.h>',
      '',
      'INICIO',
      '    INTEIRO i = 0;',
      '    INTEIRO soma = 0;',
      '',
      '    PARA (i, 1, 5) {',
      '        soma = soma + i;',
      '        ESCREVA_FMT("i=%d soma=%d\\n", i, soma);',
      '    }',
      '    ESCREVAL("fim");',
      'FIM',
      '',
    ].join('\n');
    await avaliar(`(() => {
      window.__basicgol.editor.setValue(${JSON.stringify(programa)}, -1);
      window.__basicgol.depurador.limparPontos();
      const t = document.querySelector('#debug-toggle');
      t.checked = true;
      t.dispatchEvent(new Event('change'));
      return 1;
    })()`);
    await dormir(200);
    conferir(await avaliar(`document.body.classList.contains('depurando')
      && !document.querySelector('#debug-pane').classList.contains('oculto')`),
    'ligar o depurador mostra o painel e vira o botão para Debug');

    const esperarParado = async (tentativas = 60) => {
      for (let i = 0; i < tentativas; i += 1) {
        await dormir(400);
        if (await avaliar('window.__basicgol.depurador.estaParado()')) return true;
      }
      return false;
    };

    await avaliar(`document.querySelector('#run-btn').click(), 1`);
    const parouNoInicio = await esperarParado();
    conferir(parouNoInicio && await avaliar('window.__basicgol.depurador.linha') === 4,
      'sem pontos de parada, o programa para na primeira linha instrumentada',
      `linha ${await avaliar('window.__basicgol.depurador.linha')}`);

    // Um passo: a linha anda e as variáveis já aparecem no painel.
    await avaliar(`document.querySelector('#debug-passo').click(), 1`);
    await esperarParado();
    conferir(await avaliar('window.__basicgol.depurador.linha') === 5,
      'o botão Passo anda uma linha');
    conferir(await avaliar(`document.querySelectorAll('#debug-variaveis tr').length > 0
      && document.querySelector('#debug-variaveis .nome').textContent === 'i'`),
    'o painel mostra as variáveis visíveis');

    // Ponto de parada dentro do laço: Continuar vai até lá.
    await avaliar(`window.__basicgol.depurador.alternarPonto(9), 1`);
    await avaliar(`document.querySelector('#debug-continuar').click(), 1`);
    await esperarParado();
    const emPonto = await avaliar('window.__basicgol.depurador.linha');
    conferir(emPonto === 9, 'Continuar para no ponto de parada', `linha ${emPonto}`);
    const valores = await avaliar(`window.__basicgol.depurador.variaveis
      .map((v) => v.nome + '=' + v.valor).join(' ')`);
    conferir(/i=1/.test(valores) && /soma=1/.test(valores),
      'os valores das variáveis são os da linha onde parou', valores);

    // O marcador da linha atual existe no editor.
    conferir(await avaliar(`Object.values(window.__basicgol.editor.session.getMarkers(false) || {})
      .some((m) => m.clazz === 'linha-depuracao')`),
    'a linha onde o programa parou fica destacada no editor');

    // Parar encerra o programa.
    await avaliar(`document.querySelector('#debug-parar').click(), 1`);
    let encerrou = false;
    for (let i = 0; i < 40 && !encerrou; i += 1) {
      await dormir(300);
      encerrou = !(await avaliar('window.__basicgol.estado.rodando'));
    }
    saida = await tela();
    conferir(encerrou && /depurador encerrou o programa/.test(saida),
      'o botão Parar encerra o programa');

    // Instrumentação impossível (um `for` quebrado em duas linhas): o
    // programa ainda roda, sem o depurador, e a página diz isso.
    const dificil = [
      '#include <basicgol.h>',
      '',
      'INICIO',
      '    INTEIRO total = 0;',
      '    for (int i = 0;',
      '         i < 5; i++) {',
      '        total = total + i;',
      '    }',
      '    ESCREVAL(total);',
      'FIM',
      '',
    ].join('\n');
    await avaliar(`(() => {
      window.__basicgol.editor.setValue(${JSON.stringify(dificil)}, -1);
      window.__basicgol.depurador.limparPontos();
      document.querySelector('#run-btn').click();
      return 1;
    })()`);
    let terminou = false;
    for (let i = 0; i < 60 && !terminou; i += 1) {
      await dormir(400);
      terminou = !(await avaliar('window.__basicgol.estado.rodando'));
    }
    saida = await tela();
    conferir(terminou && /não consegui instrumentar/.test(saida) && /\b10\b/.test(saida),
      'programa que não dá para instrumentar roda mesmo assim, sem o depurador');

    // Desligar devolve a página ao estado normal.
    await avaliar(`(() => {
      window.__basicgol.depurador.limparPontos();
      const t = document.querySelector('#debug-toggle');
      t.checked = false;
      t.dispatchEvent(new Event('change'));
      return 1;
    })()`);
    await dormir(200);
    conferir(await avaliar(`document.querySelector('#debug-pane').classList.contains('oculto')
      && !document.body.classList.contains('depurando')`),
    'desligar o depurador esconde o painel');

    // 5) Nenhum erro de página no console.
    const erros = eventos
      .filter((e) => e.method === 'Runtime.exceptionThrown'
        || (e.method === 'Log.entryAdded' && e.params.entry.level === 'error'))
      .map((e) => (e.method === 'Log.entryAdded'
        ? e.params.entry.text
        : `${e.params.exceptionDetails.text} ${(e.params.exceptionDetails.exception || {}).description || ''}`));
    conferir(erros.length === 0, 'nenhum erro de console', erros.join(' | '));
  } finally {
    ws.close();
    processo.kill('SIGKILL');
    if (!process.argv.includes('--manter')) servidor.close();
  }

  if (falhas.length) {
    console.error(`\n${falhas.length} verificação(ões) falharam.`);
    process.exit(1);
  }
  console.log('\npágina verificada: compila, executa, espera a entrada, carrega os exemplos e depura.');
}

principal().catch((e) => {
  console.error(e);
  process.exit(1);
});

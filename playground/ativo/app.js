/*
 * app.js — a página do playground BASICGOL.
 *
 * Responsabilidades:
 *   - montar a interface (topbar, sidebar, editor Ace, terminal xterm);
 *   - manter o programa no localStorage (e no endereço, como ?p=);
 *   - orquestrar Executar: preparar fonte -> compilar no navegador -> rodar;
 *   - atalhos de teclado e alternância de layout, iguais aos do cplayground.
 */

import { Ferramentas, prepararFonte, lerZip } from './ferramentas.js';
import { Terminal, temJspi } from './terminal.js';
import { criarWasiPrograma, DecodificadorSeguro } from './armazenamento.js';
import { EXEMPLOS, acharExemplo } from './exemplos.js';

const CHAVE_ARMAZENAMENTO = 'basicgol.playground.programa';
const CHAVE_LAYOUT = 'basicgol.playground.layout';
const CHAVE_TEMA = 'basicgol.playground.tema';

const PROGRAMA_INICIAL = `// Bem-vindo ao playground BASICGOL!
//
// Isto e C compilado por clang/WebAssembly dentro do seu navegador.
// O cabecalho <basicgol.h> instala as macros da linguagem:
// INICIO/FIM, SE/SENAO, PARA, ENQUANTO, ESCREVA/LEIA e os tipos.

#include <basicgol.h>

INICIO
    TEXTO nome = "mundo";
    INTEIRO i = 0;

    ESCREVAL("BASICGOL no navegador: nenhum servidor compila o seu codigo.");

    PARA (i, 1, 3) {
        ESCREVA("  iteracao ");
        ESCREVAL(i);
    }

    ESCREVA("Digite o seu nome: ");
    TEXTO_FIXO(entrada, 64);
    SE LEIA_TEXTO(entrada, TAMANHO_VETOR(entrada)) ENTAO
        nome = entrada;
    FIM_SE

    ESCREVA("Ola, ");
    ESCREVA(nome);
    ESCREVAL("! Bom estudo de BASICGOL.");
FIM
`;

/** Estado da interface. */
const estado = {
  layout: 'split', // 'split' | 'code' | 'term'
  sidebar: false,
  tema: 'padrao',
  rodando: false,
  arquivos: [], // arquivos enviados pelo usuário ({ nome, bytes })
};

const $ = (seletor) => document.querySelector(seletor);

let editor;
let terminal;
let ferramentas;

/* ------------------------------------------------------------------ ícones */

function montarIcones(icones) {
  const svgNS = 'http://www.w3.org/2000/svg';
  document.querySelectorAll('[data-icone]').forEach((el) => {
    const icone = icones[el.dataset.icone];
    if (!icone) return;
    const svg = document.createElementNS(svgNS, 'svg');
    svg.setAttribute('viewBox', `0 0 ${icone.w} ${icone.h}`);
    svg.setAttribute('class', 'icone');
    svg.setAttribute('aria-hidden', 'true');
    const caminho = document.createElementNS(svgNS, 'path');
    caminho.setAttribute('d', icone.d);
    svg.appendChild(caminho);
    // Mantém as classes/atributos do elemento original (ex.: o ícone dentro de
    // um botão que tem hover/estado próprio).
    el.replaceWith(svg);
  });
}

/* ------------------------------------------------------------------- editor */

function criarEditor() {
  // eslint-disable-next-line no-undef
  const aceEditor = window.ace.edit('editor');
  aceEditor.session.setMode('ace/mode/c_cpp');
  aceEditor.session.setUseWrapMode(true);
  aceEditor.session.setTabSize(4);
  aceEditor.session.setUseSoftTabs(true);
  aceEditor.setTheme('ace/theme/textmate');
  aceEditor.setOption('fontFamily', 'Ubuntu Mono');
  aceEditor.setOption('fontSize', '12pt');
  aceEditor.setOption('showPrintMargin', false);
  aceEditor.setOption('highlightActiveLine', true);
  aceEditor.setAutoScrollEditorIntoView(true);
  aceEditor.$blockScrolling = Infinity;
  aceEditor.commands.addCommand({
    name: 'executar',
    bindKey: { win: 'Shift-Enter', mac: 'Shift-Enter' },
    exec: () => executar(),
  });
  aceEditor.commands.addCommand({
    name: 'configuracoes',
    bindKey: { win: 'Ctrl-,', mac: 'Command-,' },
    exec: () => alternarSidebar(),
  });
  return aceEditor;
}

function programaSalvo() {
  try {
    return localStorage.getItem(CHAVE_ARMAZENAMENTO) || PROGRAMA_INICIAL;
  } catch (e) {
    return PROGRAMA_INICIAL;
  }
}

function salvarPrograma(texto) {
  try {
    localStorage.setItem(CHAVE_ARMAZENAMENTO, texto);
  } catch (e) { /* modo privado: segue sem persistir */ }
}

/* ------------------------------------------------------------------- layout */

function aplicarLayout() {
  const container = $('#primary-container');
  container.classList.toggle('open-sidebar', estado.sidebar);
  container.classList.toggle('show-code-only', estado.layout === 'code');
  container.classList.toggle('show-term-only', estado.layout === 'term');
  document.body.classList.toggle('show-split', estado.layout === 'split');
  try { localStorage.setItem(CHAVE_LAYOUT, estado.layout); } catch (e) { /* ok */ }
  setTimeout(() => {
    editor.resize();
    terminal.centralizar();
  }, 320);
}

function alternarSidebar() {
  estado.sidebar = !estado.sidebar;
  aplicarLayout();
}

function definirLayout(novo) {
  estado.layout = novo;
  aplicarLayout();
}

function aplicarTema(nome) {
  estado.tema = nome;
  document.documentElement.dataset.tema = nome;
  terminal.definirPaleta(nome);
  try { localStorage.setItem(CHAVE_TEMA, nome); } catch (e) { /* ok */ }
  const seletor = $('#theme-select');
  if (seletor) seletor.value = nome;
}

/* ------------------------------------------------------------------- topbar */

function definirRodando(rodando) {
  estado.rodando = rodando;
  $('#run-btn').classList.toggle('disabled', rodando);
}

/* ------------------------------------------------------------------ execução */

/** Escreve uma linha do log da compilação no terminal, colorida por tipo. */
function registrarCompilacao(texto) {
  String(texto).split('\n').forEach((linha) => {
    if (!linha.trim()) return;
    if (/warning|aviso/i.test(linha)) terminal.escreverCru(`\x1b[93m${linha}\x1b[0m\r\n`);
    else if (/error|erro/i.test(linha)) terminal.escreverCru(`\x1b[91m${linha}\x1b[0m\r\n`);
    else terminal.escreverCru(`\x1b[90m${linha}\x1b[0m\r\n`);
  });
}

function flagsSelecionadas() {
  const marcadas = [...document.querySelectorAll('#compiler-flags input[type="checkbox"]:checked')]
    .map((c) => c.value);
  const avisos = marcadas.filter((f) => f.startsWith('-W') || f === '-pedantic');
  // O compilador "clang + avisos" é o -Wall ligado (como no cplayground).
  const comAvisos = $('#compiler-select').value === 'clang-wall'
    && !avisos.includes('-Wall');
  return comAvisos ? [...avisos, '-Wall'] : avisos;
}

async function executar() {
  if (estado.rodando) return;
  definirRodando(true);
  terminal.limpar();
  terminal.limparRoteiro();

  const codigo = editor.getValue();
  salvarPrograma(codigo);
  if (estado.arquivos.length) terminal.detalhe(`${estado.arquivos.length} arquivo(s) de apoio incluído(s).`);

  terminal.detalhe('compilando no navegador (clang → wasm-ld)…');
  const inicio = performance.now();
  let resultado;
  try {
    resultado = await ferramentas.compilar({
      codigo: prepararFonte(codigo),
      flags: flagsSelecionadas(),
      otimizacao: $('#compiler-optimization').value,
      arquivos: estado.arquivos,
      aoRegistrar: (linha) => registrarCompilacao(linha),
    });
  } catch (e) {
    terminal.erro(`falha ao compilar: ${e.message}`);
    definirRodando(false);
    return;
  }

  if (!resultado.sucesso) {
    terminal.erro('compilação falhou. Corrija o programa e tente de novo.');
    definirRodando(false);
    return;
  }
  terminal.detalhe(`compilado em ${((performance.now() - inicio) / 1000).toFixed(2)}s`);
  await executarWasm(resultado.wasm);
}

async function executarWasm(wasm) {
  // Com JSPI, fd_read pode devolver Promise e o programa "espera" o Enter.
  // Sem JSPI, a entrada é digitada de antemão e lida como um fluxo (roteiro).
  let interativo = temJspi();
  if (!interativo) {
    if (terminal.roteiroVazio()) {
      terminal.detalhe('este navegador ainda não tem JSPI: digite as entradas '
        + 'no terminal antes de executar (o programa lê o que você digitar).');
    } else {
      terminal.detalhe('entrada não interativa: o programa lê o que você digitou.');
    }
  }

  let memoria = null;
  const decodificador = new DecodificadorSeguro();
  const io = {
    escrever: (bytes, canal) => {
      const texto = decodificador.empurrar(bytes);
      if (!texto) return;
      if (canal === 'erro') terminal.escreverCru(`\x1b[91m${texto.replace(/\n/g, '\r\n')}\x1b[0m`);
      else terminal.escreverCru(texto.replace(/\n/g, '\r\n'));
    },
    ler: (quantidade) => (interativo
      ? terminal.lerLinha()
      : terminal.lerRoteiro(quantidade)),
  };

  const argumentos = ($('#runtime-args').value || '').trim().split(/\s+/).filter(Boolean);
  const imports = criarWasiPrograma({
    obterMemoria: () => memoria,
    io,
    argv: ['programa', ...argumentos],
  });

  if (interativo) {
    // Embrulha cada import que pode devolver Promise no Suspending: é o que
    // permite a instância ser suspensa e retomada na mesma instrução.
    for (const chave of Object.keys(imports)) {
      if (typeof imports[chave] !== 'function') continue;
      const original = imports[chave];
      imports[chave] = (...args) => {
        const r = original(...args);
        return (r && typeof r.then === 'function') ? new WebAssembly.Suspending(() => r) : r;
      };
    }
  }

  let instancia;
  try {
    const modulo = await WebAssembly.compile(wasm);
    instancia = await WebAssembly.instantiate(modulo, {
      wasi_unstable: imports,
      wasi_snapshot_preview1: imports,
    });
  } catch (e) {
    terminal.erro(`não consegui iniciar o programa: ${e.message}`);
    definirRodando(false);
    return;
  }
  memoria = instancia.exports.memory;

  const inicio = performance.now();
  let codigoSaida = 0;
  try {
    if (interativo) {
      await WebAssembly.promising(instancia.exports._start)();
    } else {
      instancia.exports._start();
    }
  } catch (e) {
    if (e && typeof e.codigoSaida === 'number') {
      codigoSaida = e.codigoSaida;
    } else if (e instanceof WebAssembly.RuntimeError) {
      terminal.escreverCru('\r\n');
      terminal.erro(`o programa abortou (${e.message}).`);
      codigoSaida = 134;
    } else if (e && e.message && /suspend|promise|suspending/i.test(e.message)) {
      terminal.erro('este navegador não aceita entrada interativa; '
        + 'digite a entrada antes de executar.');
      codigoSaida = 1;
    } else {
      terminal.erro(`erro inesperado: ${e && e.message}`);
      codigoSaida = 1;
    }
  }

  // Se o programa saiu mas a instância seguia suspensa (EOF no meio de uma
  // leitura), resolvemos a promessa pendente para liberar a memória.
  if (terminal.emEdicao && terminal.aguardando) {
    terminal.emEdicao = false;
    const pendente = terminal.aguardando;
    terminal.aguardando = null;
    pendente(new Uint8Array(0));
  }

  const ms = Math.round(performance.now() - inicio);
  terminal.escreverCru('\r\n');
  if (codigoSaida === 0) terminal.detalhe(`[o programa terminou em ${ms} ms]`);
  else terminal.detalhe(`[o programa terminou com código ${codigoSaida} em ${ms} ms]`);
  definirRodando(false);
}

/* ---------------------------------------------------------------- exemplos */

function montarExemplos() {
  const painel = document.createElement('div');
  painel.className = 'lista-exemplos oculto';
  painel.id = 'lista-exemplos';
  const lista = document.createElement('ul');
  for (const exemplo of EXEMPLOS) {
    const item = document.createElement('li');
    const botao = document.createElement('button');
    botao.type = 'button';
    botao.textContent = exemplo.titulo;
    botao.title = exemplo.arquivo;
    botao.addEventListener('click', () => {
      carregarExemplo(exemplo);
      painel.classList.add('oculto');
      alternarExemplos(false);
    });
    item.appendChild(botao);
    lista.appendChild(item);
  }
  painel.appendChild(lista);
  document.body.appendChild(painel);
}

function alternarExemplos(forcar) {
  const painel = $('#lista-exemplos');
  const abrir = forcar === undefined ? painel.classList.contains('oculto') : forcar;
  painel.classList.toggle('oculto', !abrir);
}

function carregarExemplo(exemplo) {
  editor.setValue(exemplo.codigo, -1);
  editor.clearSelection();
  salvarPrograma(exemplo.codigo);
  terminal.limpar();
  terminal.limparRoteiro();
  // Os exemplos com entrada dourada já vêm com ela no roteiro: sem JSPI, é o
  // que permite "pôr o programa para rodar" e ver a saída esperada.
  if (exemplo.entrada) {
    terminal.adicionarAoRoteiro(exemplo.entrada);
    terminal.limpar();
    terminal.detalhe(`exemplo carregado: ${exemplo.titulo}`);
    terminal.detalhe('a entrada de teste deste exemplo já está no terminal.');
  } else {
    terminal.detalhe(`exemplo carregado: ${exemplo.titulo}`);
  }
}

/* ----------------------------------------------------------------- arquivos */

async function aoEnviarZip(evento) {
  const arquivo = evento.target.files[0];
  evento.target.value = '';
  if (!arquivo) return;
  const erroEl = $('#upload-error');
  erroEl.textContent = '';
  if (arquivo.size > 1024 * 1024) {
    erroEl.textContent = 'O arquivo é grande demais. Máximo: 1 MB.';
    return;
  }
  $('#upload-spinner').classList.remove('oculto');
  try {
    const bytes = new Uint8Array(await arquivo.arrayBuffer());
    const lista = await lerZip(bytes);
    estado.arquivos = lista;
    ferramentas.definirArquivosUsuario(lista);
    $('#uploaded-filename').textContent = `${arquivo.name} (${lista.length} arquivos)`;
  } catch (e) {
    erroEl.textContent = e.message;
  } finally {
    $('#upload-spinner').classList.add('oculto');
  }
}

function removerZip() {
  estado.arquivos = [];
  if (ferramentas) ferramentas.definirArquivosUsuario([]);
  $('#uploaded-filename').textContent = '';
  $('#upload-error').textContent = '';
}

/* ------------------------------------------------------------- inicialização */

async function iniciar() {
  // Tema e layout salvos (ou ?theme=, como no original).
  const params = new URLSearchParams(location.search);
  const tema = params.get('theme') || localStorage.getItem(CHAVE_TEMA) || 'padrao';
  const temaValido = ['padrao', 'monokai', 'zenburn'].includes(tema) ? tema : 'padrao';
  estado.layout = localStorage.getItem(CHAVE_LAYOUT) || 'split';
  if (!['split', 'code', 'term'].includes(estado.layout)) estado.layout = 'split';
  estado.sidebar = params.get('settings') === '1';

  // Ícones (SVG local, extraído do Font Awesome 5.6.3 — o mesmo do original).
  try {
    const resposta = await fetch(new URL('../vendor/fa/icones.json', import.meta.url));
    montarIcones(await resposta.json());
  } catch (e) {
    console.warn('ícones não carregaram:', e);
  }

  editor = criarEditor();
  let inicial = programaSalvo();
  if (params.get('p')) {
    try {
      inicial = decodeURIComponent(escape(atob(params.get('p'))));
    } catch (e) { /* ?p inválido: ignora */ }
  }
  editor.setValue(inicial, -1);
  editor.clearSelection();
  editor.session.on('change', () => salvarPrograma(editor.getValue()));

  terminal = new Terminal(document.getElementById('terminal'), {
    aoDigitar: (dados) => {
      if (!estado.rodando) {
        // Fora de execução, digitar é montar o roteiro de entrada.
        terminal.adicionarAoRoteiro(dados);
        return;
      }
      if (temJspi() && terminal.emEdicao) terminal.processarTeclaInterativa(dados);
      else if (!temJspi()) terminal.adicionarAoRoteiro(dados);
    },
  });

  aplicarTema(temaValido);
  aplicarLayout();

  ferramentas = new Ferramentas({
    aoProgredir: ({ mensagem }) => {
      $('#toolchain-status').innerHTML = `<small>${mensagem}</small>`;
    },
    aoTraco: (linha) => console.info('[BASICGOL]', linha),
  });
  ferramentas.carregar().then(() => {
    $('#toolchain-status').innerHTML = '<small>Ferramentas prontas. Pressione Run.</small>';
    $('#toolchain-detalhe').textContent = temJspi()
      ? 'entrada interativa: sim (JSPI)'
      : 'entrada interativa: digite antes de executar';
  }).catch((e) => {
    $('#toolchain-status').innerHTML = '<small>Falha ao carregar as ferramentas.</small>';
    $('#toolchain-detalhe').textContent = e.message;
    terminal.erro(e.message);
  });

  // --- topbar
  $('#run-btn').addEventListener('click', () => executar());
  $('#stop-btn').addEventListener('click', () => {
    if (!estado.rodando) return;
    terminal.erro('interrompido.');
    // Recarregar a página é a única forma de matar o wasm em execução.
    window.location.reload();
  });
  $('#settings-btn').addEventListener('click', alternarSidebar);
  $('#limpar-btn').addEventListener('click', () => {
    terminal.limpar();
    terminal.limparRoteiro();
  });
  $('#terminal-btn').addEventListener('click', () => {
    definirLayout(estado.layout === 'term' ? 'split' : 'term');
  });
  montarExemplos();
  $('#examples-btn').addEventListener('click', () => alternarExemplos());
  document.addEventListener('click', (evento) => {
    const painel = $('#lista-exemplos');
    if (painel.classList.contains('oculto')) return;
    if (painel.contains(evento.target) || evento.target.closest('#examples-btn')) return;
    alternarExemplos(false);
  });

  // --- sidebar
  $('#theme-select').addEventListener('change', (e) => aplicarTema(e.target.value));
  $('#input-include-file').addEventListener('change', aoEnviarZip);
  $('#btn-remove-uploaded-file').addEventListener('click', removerZip);

  // --- teclado global (como no original: shift+enter, cmd+, cmd+e)
  document.addEventListener('keydown', (evento) => {
    const mac = /Mac|iPhone|iPad/.test(navigator.platform);
    const meta = mac ? evento.metaKey : evento.ctrlKey;
    if (evento.key === 'Enter' && evento.shiftKey) {
      evento.preventDefault();
      executar();
      return;
    }
    if (meta && evento.key === ',') {
      evento.preventDefault();
      alternarSidebar();
      return;
    }
    if (meta && evento.key === 'e') {
      evento.preventDefault();
      alternarExemplos();
      return;
    }
    if (meta && evento.key === '.') {
      evento.preventDefault();
      if (estado.rodando) window.location.reload();
      return;
    }
    if (evento.key === 'Escape') alternarExemplos(false);
  });

  // Ajusta o editor/terminal quando a janela muda de tamanho.
  window.addEventListener('resize', () => {
    editor.resize();
    terminal.centralizar();
  });

  terminal.detalhe('BASICGOL playground — compile e rode no seu navegador.');
}

document.addEventListener('DOMContentLoaded', iniciar);

// Exportado para depuração no console.
window.__basicgol = {
  get editor() { return editor; },
  get terminal() { return terminal; },
  get ferramentas() { return ferramentas; },
  get estado() { return estado; },
  executar,
  acharExemplo,
};

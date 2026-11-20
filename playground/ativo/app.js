/*
 * app.js — a página do playground BASICGOL.
 *
 * Responsabilidades:
 *   - montar a interface (topbar, sidebar, editor Ace, terminal xterm);
 *   - manter o programa no localStorage (e no endereço, como ?p=);
 *   - orquestrar Executar: preparar fonte -> compilar no navegador -> rodar;
 *   - atalhos de teclado e alternância de layout, iguais aos do cplayground.
 */

import { Ferramentas, prepararFonte, lerZip, dicasPara } from './ferramentas.js';
import { Terminal, temJspi } from './terminal.js';
import { criarWasiPrograma, DecodificadorSeguro } from './armazenamento.js';
import { EXEMPLOS, acharExemplo } from './exemplos.js';
import { Depurador, instrumentar, SAIDA_PARADO } from './depurador.js';

const CHAVE_ARMAZENAMENTO = 'basicgol.playground.programa';
const CHAVE_LAYOUT = 'basicgol.playground.layout';
const CHAVE_TEMA = 'basicgol.playground.tema';
const CHAVE_DEPURADOR = 'basicgol.playground.depurador';
const CHAVE_PONTOS = 'basicgol.playground.pontos';

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
let depurador;
let marcadorLinha = null; // marcador Ace da linha onde o programa parou

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
    // O `<i>` pode ser um botão por si só (`#btn-remove-uploaded-file`): todos
    // os atributos e classes do original passam para o SVG, senão o id, o
    // tabindex e o cursor se perdem junto com o elemento substituído.
    for (const atributo of Array.from(el.attributes)) {
      if (atributo.name === 'class') continue;
      svg.setAttribute(atributo.name, atributo.value);
    }
    svg.setAttribute('class', ['icone', ...el.classList].join(' '));
    for (const classe of Array.from(el.classList)) svg.classList.add(classe);
    for (const filho of Array.from(el.childNodes)) svg.appendChild(filho);
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
  const embutido = estado.layout !== 'split';
  container.classList.toggle('open-sidebar', estado.sidebar);
  container.classList.toggle('show-code-only', estado.layout === 'code');
  container.classList.toggle('show-term-only', estado.layout === 'term');
  document.body.classList.toggle('show-split', !embutido);
  document.body.classList.toggle('embedded', embutido);
  // Os botões do modo embutido são os mesmos do cplayground.
  for (const seletor of ['#edit-btn', '#split-pane-btn', '#open-in-basicgol-btn']) {
    const botao = $(seletor);
    if (botao) botao.classList.toggle('oculto', !embutido);
  }
  const editar = $('#edit-btn');
  if (editar) editar.classList.toggle('oculto', !embutido || estado.layout === 'code');
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
  terminal.definirPaleta();
  const seletor = $('#theme-select');
  if (seletor) seletor.value = nome;
}

/* ------------------------------------------------------------------- topbar */

function definirRodando(rodando) {
  estado.rodando = rodando;
  $('#run-btn').classList.toggle('disabled', rodando);
  if (depurador) desenharDepurador();
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

/**
 * Uma passada do compilador. `guardar` segura o log em vez de escrevê-lo no
 * terminal: é o que permite tentar o fonte instrumentado e, se ele não
 * compilar, mostrar só os erros do programa original (ver `executar`).
 */
async function compilarFonte(fonte, preparado, { depurar = false, guardar = false, aoGerarC = null } = {}) {
  const guardado = [];
  try {
    const resultado = await ferramentas.compilar({
      codigo: fonte,
      deslocamentoLinhas: preparado.deslocamento,
      flags: flagsSelecionadas(),
      otimizacao: $('#compiler-optimization').value,
      arquivos: estado.arquivos,
      depurar,
      aoGerarC,
      aoRegistrar: (linha) => (guardar ? guardado.push(linha) : registrarCompilacao(linha)),
    });
    return { ...resultado, guardado };
  } catch (e) {
    return { sucesso: false, registro: '', guardado, excecao: e };
  }
}

async function executar() {
  if (estado.rodando) return;
  definirRodando(true);
  terminal.limpar();

  const codigo = editor.getValue();
  salvarPrograma(codigo);
  if (estado.arquivos.length) terminal.detalhe(`${estado.arquivos.length} arquivo(s) de apoio incluído(s).`);

  const preparado = prepararFonte(codigo);
  let depurando = depurador.ativo;
  const instrumentada = depurando ? instrumentar(preparado.fonte) : null;
  if (depurando) anunciarDepurador(instrumentada);

  terminal.detalhe('compilando no navegador (clang → wasm-ld)…');
  const inicio = performance.now();

  const mostrarC = $('#show-c').checked;
  const previa = $('#generated-c');
  previa.textContent = mostrarC ? 'Gerando C…' : 'Pressione Run para gerar o C.';

  let resultado = await compilarFonte(
    depurando ? instrumentada.fonte : preparado.fonte,
    preparado,
    {
      depurar: depurando,
      guardar: depurando,
      aoGerarC: mostrarC ? (c) => { previa.textContent = c; } : null,
    },
  );
  if (depurando && resultado.sucesso) {
    // Compilou instrumentado: o log guardado (avisos) vai para o terminal.
    for (const linha of resultado.guardado) registrarCompilacao(linha);
  } else if (depurando) {
    // Pode ser o programa, pode ser a instrumentação: a única forma honesta
    // de saber é compilar o original — e é o log dele que o usuário vê.
    const semGanchos = await compilarFonte(preparado.fonte, preparado);
    if (semGanchos.sucesso) {
      terminal.destaque('não consegui instrumentar este programa; ele vai rodar sem o depurador.');
      depurando = false;
    }
    resultado = semGanchos;
  }

  if (resultado.excecao) {
    if (mostrarC) previa.textContent = 'Não foi possível gerar o C.';
    terminal.erro(`falha ao compilar: ${resultado.excecao.message}`);
    definirRodando(false);
    return;
  }

  if (mostrarC && previa.textContent === 'Gerando C…') previa.textContent = 'Não foi possível gerar o C.';
  if (!resultado.sucesso) {
    for (const dica of dicasPara(codigo, resultado.registro)) {
      terminal.detalhe(`dica: ${dica}`);
    }
    terminal.erro('compilação falhou. Corrija o programa e tente de novo.');
    definirRodando(false);
    return;
  }
  terminal.detalhe(`compilado em ${((performance.now() - inicio) / 1000).toFixed(2)}s`);
  await executarWasm(resultado.wasm, {
    depurando,
    deslocamento: preparado.deslocamento,
    linhasValidas: instrumentada ? instrumentada.linhas : null,
  });
}

async function executarWasm(wasm, { depurando = false, deslocamento = 0, linhasValidas = null } = {}) {
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
    ler: (quantidade) => {
      // Primeiro o que já foi digitado (ou a entrada de teste do exemplo);
      // só quando acaba é que a leitura passa a esperar o teclado.
      const pronta = terminal.lerRoteiro(quantidade);
      if (pronta.length || !interativo) return pronta;
      return terminal.lerLinha();
    },
  };

  if (depurando) {
    depurador.iniciar({ deslocamento, podePausar: interativo, linhasValidas });
  }

  const argumentos = ($('#runtime-args').value || '').trim().split(/\s+/).filter(Boolean);
  const imports = criarWasiPrograma({
    obterMemoria: () => memoria,
    io,
    argv: ['programa', ...argumentos],
    // O descritor 3 é o canal do depurador; sem ele, o programa instrumentado
    // percebe o BADF e se desliga sozinho.
    depuracao: depurando ? depurador.canal() : null,
  });

  if (interativo) {
    // `fd_read` é o único import que pode demorar (espera o Enter). Com JSPI
    // ele é *envolto* num `WebAssembly.Suspending`: o import passa a devolver
    // sempre uma Promise, e o navegador suspende a instância até ela resolver,
    // retomando na mesma instrução. (Envolver o valor de retorno não funciona:
    // quem precisa ser "suspending" é o import, não o resultado.)
    const lerOriginal = imports.fd_read;
    imports.fd_read = new WebAssembly.Suspending(
      (...args) => Promise.resolve(lerOriginal(...args)),
    );
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

  // O foco pode ter ido para o terminal durante uma leitura: devolve ao editor.
  if (editor && typeof editor.focus === 'function') editor.focus();

  const ms = Math.round(performance.now() - inicio);
  terminal.escreverCru('\r\n');
  if (depurando && depurador.pedidoParar && codigoSaida === SAIDA_PARADO) {
    terminal.detalhe(`[o depurador encerrou o programa em ${ms} ms]`);
  } else if (codigoSaida === 0) {
    terminal.detalhe(`[o programa terminou em ${ms} ms]`);
  } else {
    terminal.detalhe(`[o programa terminou com código ${codigoSaida} em ${ms} ms]`);
  }
  if (depurando) {
    const quantas = depurador.eventos;
    if (depurador.aviso) terminal.destaque(`depurador: ${depurador.aviso}`);
    terminal.detalhe(`depurador: ${quantas} linha(s) percorrida(s).`);
    depurador.encerrar();
  }
  definirRodando(false);
}

/* --------------------------------------------------------------- depurador */

/** Diz no terminal, antes de compilar, o que esperar desta execução. */
function anunciarDepurador(instrumentada) {
  const pontos = depurador.pontos.size;
  terminal.detalhe(pontos
    ? `depurador ligado: ${pontos} ponto(s) de parada em ${instrumentada.linhas.length} `
      + 'linha(s) instrumentada(s).'
    : `depurador ligado: sem pontos de parada, o programa para na primeira das `
      + `${instrumentada.linhas.length} linha(s) instrumentada(s).`);
  if (!temJspi()) {
    terminal.destaque('sem JSPI neste navegador o programa não consegue congelar: '
      + 'o depurador só registra as linhas e os valores (sem parar).');
  }
}

/** Aplica o interruptor: painel, face do botão Run e medianiz do editor. */
function aplicarDepurador() {
  const ligado = depurador.ativo;
  document.body.classList.toggle('depurando', ligado);
  $('#debug-pane').classList.toggle('oculto', !ligado);
  $('#debug-toggle').checked = ligado;
  try { localStorage.setItem(CHAVE_DEPURADOR, ligado ? '1' : '0'); } catch (e) { /* ok */ }
  desenharDepurador();
  // O painel toma espaço do terminal: o xterm precisa refazer as contas.
  setTimeout(() => terminal.centralizar(), 60);
}

/** Linha onde o programa parou, destacada no editor. */
function desenharMarcador() {
  if (marcadorLinha !== null) {
    editor.session.removeMarker(marcadorLinha);
    marcadorLinha = null;
  }
  if (!depurador.estaParado()) return;
  const { Range } = window.ace.require('ace/range');
  const linha = Math.max(0, depurador.linha - 1);
  marcadorLinha = editor.session.addMarker(
    new Range(linha, 0, linha, 1), 'linha-depuracao', 'fullLine',
  );
  editor.scrollToLine(linha, true, true, () => {});
}

/** Pontos de parada no medianiz (a verdade está em `depurador.pontos`). */
function desenharPontos() {
  editor.session.clearBreakpoints();
  if (!depurador.ativo) return;
  for (const linha of depurador.pontos) editor.session.setBreakpoint(linha - 1, 'ace_breakpoint');
  const quantos = depurador.pontos.size;
  const aviso = $('#debug-pontos');
  aviso.classList.toggle('oculto', quantos === 0);
  aviso.firstElementChild.textContent = quantos === 1
    ? '1 ponto de parada marcado.'
    : `${quantos} pontos de parada marcados.`;
  $('#debug-limpar').classList.toggle('oculto', quantos === 0);
}

function desenharVariaveis() {
  const corpo = $('#debug-variaveis');
  const vazio = $('#debug-vazio');
  corpo.textContent = '';
  const lista = depurador.variaveis;
  if (!lista.length) {
    vazio.classList.remove('oculto');
    vazio.textContent = depurador.estaParado()
      ? 'nenhuma variável visível nesta linha ainda.'
      : 'as variáveis aparecem quando o programa parar.';
    return;
  }
  vazio.classList.add('oculto');
  for (const variavel of lista) {
    const linha = document.createElement('tr');
    for (const [classe, texto] of [['nome', variavel.nome], ['tipo', variavel.tipo],
      ['valor', variavel.valor]]) {
      const celula = document.createElement('td');
      celula.className = classe;
      celula.textContent = texto;
      linha.appendChild(celula);
    }
    corpo.appendChild(linha);
  }
}

/** Redesenha tudo que depende do estado do depurador (chamado por `aoMudar`). */
function desenharDepurador() {
  const parado = depurador.estaParado();
  $('#debug-continuar').classList.toggle('disabled', !parado);
  $('#debug-passo').classList.toggle('disabled', !parado);
  $('#debug-parar').classList.toggle('disabled', !parado && !estado.rodando);

  let texto;
  if (parado) texto = `parado na linha ${depurador.linha}`;
  else if (estado.rodando) {
    texto = depurador.eventos ? `executando — linha ${depurador.linha}` : 'executando';
  } else if (depurador.pontos.size) texto = `${depurador.pontos.size} ponto(s) de parada`;
  else texto = 'pressione Debug para começar';
  if (depurador.aviso) texto = `${texto} · ${depurador.aviso}`;
  $('#debug-estado').textContent = texto;

  desenharVariaveis();
  desenharMarcador();
  desenharPontos();
}

function salvarPontos() {
  try {
    localStorage.setItem(CHAVE_PONTOS, JSON.stringify([...depurador.pontos]));
  } catch (e) { /* modo privado: segue sem persistir */ }
}

function carregarPontos() {
  try {
    const salvos = JSON.parse(localStorage.getItem(CHAVE_PONTOS) || '[]');
    if (Array.isArray(salvos)) {
      for (const linha of salvos) {
        if (Number.isInteger(linha) && linha > 0) depurador.pontos.add(linha);
      }
    }
  } catch (e) { /* lixo no armazenamento: começa sem pontos */ }
}

/** Botão Parar: encerra o programa, mesmo se ele estiver esperando o teclado. */
function pararDepuracao() {
  if (!estado.rodando) return;
  depurador.parar();
  if (terminal.emEdicao && terminal.aguardando) {
    const pendente = terminal.aguardando;
    terminal.emEdicao = false;
    terminal.aguardando = null;
    pendente(new Uint8Array(0)); // EOF: solta a leitura para o programa sair
  }
}

/** Liga os botões do painel, o interruptor e os cliques no medianiz. */
function montarDepurador() {
  depurador = new Depurador({ aoMudar: () => desenharDepurador() });
  carregarPontos();
  try {
    depurador.ativo = localStorage.getItem(CHAVE_DEPURADOR) === '1';
  } catch (e) { /* sem armazenamento: desligado, como o padrão */ }
  if (new URLSearchParams(location.search).get('debug') === '1') depurador.ativo = true;

  $('#debug-toggle').addEventListener('change', (evento) => {
    depurador.definirAtivo(evento.target.checked);
    aplicarDepurador();
  });
  $('#debug-continuar').addEventListener('click', () => depurador.continuar());
  $('#debug-passo').addEventListener('click', () => depurador.passo());
  $('#debug-parar').addEventListener('click', () => pararDepuracao());
  $('#debug-limpar').addEventListener('click', (evento) => {
    evento.preventDefault();
    depurador.limparPontos();
    salvarPontos();
  });

  // Clicar no número da linha marca/desmarca um ponto de parada.
  editor.on('guttermousedown', (evento) => {
    if (!depurador.ativo) return;
    const alvo = evento.domEvent.target;
    if (!alvo.className || !String(alvo.className).includes('ace_gutter-cell')) return;
    depurador.alternarPonto(evento.getDocumentPosition().row + 1);
    salvarPontos();
    evento.stop();
  });

  aplicarDepurador();
}

/* ---------------------------------------------------------------- exemplos */

/**
 * Preenche o seletor de exemplos com os programas de `docs/exemplos/`.
 * Escolher um deles troca o código do editor e põe no roteiro a entrada de
 * teste do exemplo (a mesma dos testes dourados).
 */
function montarExemplos() {
  const seletor = $('#examples-select');
  for (const exemplo of EXEMPLOS) {
    const opcao = document.createElement('option');
    opcao.value = exemplo.arquivo;
    opcao.textContent = exemplo.titulo;
    seletor.appendChild(opcao);
  }
  seletor.addEventListener('change', (evento) => {
    const exemplo = EXEMPLOS.find((e) => e.arquivo === evento.target.value);
    if (!exemplo) return;
    carregarExemplo(exemplo);
    evento.target.value = ''; // volta ao "Escolha um exemplo…"
  });
}

function carregarExemplo(exemplo) {
  editor.setValue(exemplo.codigo, -1);
  editor.clearSelection();
  salvarPrograma(exemplo.codigo);
  terminal.limparRoteiro();
  terminal.limpar();
  // A entrada de teste do exemplo (a mesma dos testes dourados) fica pronta no
  // terminal: é o que permite apertar Run e ver a saída esperada, sem digitar.
  if (exemplo.entrada) terminal.adicionarAoRoteiro(exemplo.entrada, { ecoar: false });
  terminal.detalhe(`exemplo carregado: ${exemplo.titulo}`);
  if (exemplo.entrada) {
    terminal.detalhe(`entrada de teste pronta: ${JSON.stringify(exemplo.entrada)}`);
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
  let salvo;
  try {
    const prefixo = CHAVE_TEMA + '=';
    salvo = document.cookie.split(';').map(c => c.trim()).find(c => c.startsWith(prefixo))?.slice(prefixo.length);
  } catch (e) { /* Cookies podem estar bloqueados no modo embutido. */ }
  const tema = params.get('theme') || salvo;
  const temaValido = Array.from($('#theme-select').options).some(op => op.value === tema) ? tema : 'padrao';
  // Layout: `?layout=code|term` (modo embutido do cplayground) manda mais que o
  // que ficou salvo, que manda mais que o padrão de tela dividida.
  estado.layout = params.get('layout') || localStorage.getItem(CHAVE_LAYOUT) || 'split';
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
  montarDepurador();

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

  // --- topbar (só os botões do cplayground: configurações e Run)
  $('#run-btn').addEventListener('click', () => executar());
  $('#settings-btn').addEventListener('click', alternarSidebar);
  // No modo embutido aparecem Edit / Split / Open in BASICGOL.
  $('#edit-btn').addEventListener('click', () => definirLayout('code'));
  $('#split-pane-btn').addEventListener('click', () => definirLayout('split'));
  $('#open-in-basicgol-btn').addEventListener('click', () => {
    const base = new URL('.', location.href);
    base.searchParams.set('p', btoa(unescape(encodeURIComponent(editor.getValue()))));
    window.open(base.toString(), '_blank', 'noopener');
  });
  montarExemplos();

  // --- sidebar
  $('#show-c').addEventListener('change', (e) => {
    $('#generated-pane').hidden = !e.target.checked;
    $('.code-container').classList.toggle('com-previa', e.target.checked);
    editor.resize();
  });
  $('#theme-select').addEventListener('change', (e) => {
    aplicarTema(e.target.value);
    try {
      document.cookie = `${CHAVE_TEMA}=${e.target.value}; Max-Age=31536000; SameSite=Lax`;
    } catch (erro) { /* A troca também funciona sem cookies. */ }
  });
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
      definirLayout(estado.layout === 'code' ? 'split' : 'code');
      return;
    }
    // --- depurador (só com ele ligado; F9 marca o ponto de parada)
    if (!depurador.ativo) return;
    if (evento.key === 'F8') {
      evento.preventDefault();
      depurador.continuar();
    } else if (evento.key === 'F10') {
      evento.preventDefault();
      depurador.passo();
    } else if (evento.key === 'F9') {
      evento.preventDefault();
      depurador.alternarPonto(editor.getCursorPosition().row + 1);
      salvarPontos();
    } else if (evento.key === 'F5' && evento.shiftKey) {
      evento.preventDefault();
      pararDepuracao();
    }
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
  get depurador() { return depurador; },
  executar,
  acharExemplo,
  instrumentar,
};

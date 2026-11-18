/*
 * terminal.js — o painel da direita do playground.
 *
 * Usa o xterm.js (o mesmo do cplayground.com) com as cores da paleta atual.
 *
 * O ponto delicado é a **entrada**. `LEIA`, `LEIA_OK` e `LEIA_TEXTO` chamam
 * `scanf`/`fgets`, que eventualmente chegam em `fd_read`. Este arquivo devolve
 * uma Promise que só resolve quando o usuário aperta Enter — é o que faz o
 * programa "esperar" sem travar a página:
 *
 *   1. a chamada `fd_read` devolve uma Promise (imports assíncronos);
 *   2. o navegador suspende a instância wasm que depende dela e segue
 *      desenhando a tela (o prompt já está visível, porque o stdout não tem
 *      buffer — ver tools/shim.c);
 *   3. quando a linha fica pronta, a Promise resolve, o navegador retoma a
 *      instância exatamente naquela instrução e o `fd_read` devolve os bytes.
 *
 * Esse é o comportamento especificado de "JSPI" (JavaScript Promise
 * Integration). Onde ele não existe, o terminal cai para o modo "entrada antes
 * de rodar": o que o programa for ler é digitado de antemão (como um arquivo
 * de entrada) e a interface avisa isso claramente.
 */

const CORES_POR_PALETA = {
  padrao: {
    background: '#352e3c', foreground: '#d6dbd8', cursor: '#d6dbd8',
    black: '#29282e', red: '#a63939', green: '#87a140', yellow: '#dba858',
    blue: '#5b81a0', magenta: '#85678f', cyan: '#5e8d87', white: '#818890',
    brightBlack: '#4f495f', brightRed: '#d85b7b', brightGreen: '#92c74d',
    brightYellow: '#f0d974', brightBlue: '#7daad1', brightMagenta: '#b294bb',
    brightCyan: '#87c6be', brightWhite: '#c5c8c6',
  },
  monokai: {
    background: '#2c292d', foreground: '#fcfcfa', cursor: '#fcfcfa',
    black: '#272822', red: '#f92672', green: '#a6e22e', yellow: '#75715e',
    blue: '#66d9ef', magenta: '#ae81ff', cyan: '#a1efe4', white: '#f8f8f2',
    brightBlack: '#75715e', brightRed: '#f92672', brightGreen: '#a6e22e',
    brightYellow: '#e6db74', brightBlue: '#66d9ef', brightMagenta: '#ae81ff',
    brightCyan: '#a1efe4', brightWhite: '#f9f8f5',
  },
  zenburn: {
    background: '#3f3f3f', foreground: '#dcdccc', cursor: '#dcdccc',
    black: '#4d4d4d', red: '#705050', green: '#60b48a', yellow: '#f0dfaf',
    blue: '#506070', magenta: '#dc8cc3', cyan: '#8cd0d3', white: '#dcdccc',
    brightBlack: '#709080', brightRed: '#dca3a3', brightGreen: '#c3bf9f',
    brightYellow: '#e0cf9f', brightBlue: '#94bff3', brightMagenta: '#ec93d3',
    brightCyan: '#93e0e3', brightWhite: '#ffffff',
  },
};

const VERDE = '\x1b[92m';
const AMARELO = '\x1b[93m';
const VERMELHO = '\x1b[91m';
const AZUL = '\x1b[94m';
const CINZA = '\x1b[90m';
const NORMAL = '\x1b[0m';

/** JSPI disponível neste navegador? */
export function temJspi() {
  return typeof WebAssembly === 'object'
    && typeof WebAssembly.Suspending === 'function'
    && typeof WebAssembly.promising === 'function';
}

export class Terminal {
  constructor(elemento, { aoDigitar = () => {} } = {}) {
    this.elemento = elemento;
    this.aoDigitar = aoDigitar;
    this.xterm = new window.Terminal({
      fontFamily: 'Ubuntu Mono, monospace',
      fontSize: 16,
      cursorBlink: true,
      convertEol: true,
      theme: CORES_POR_PALETA.padrao,
      scrollback: 5000,
      allowProposedApi: true,
    });
    this.ajuste = new window.FitAddon.FitAddon();
    this.xterm.loadAddon(this.ajuste);
    this.xterm.open(elemento);
    this.ajuste.fit();

    // Linha sendo digitada (modo interativo).
    this.linha = '';
    this.emEdicao = false;
    this.aguardando = null;

    // Fila de entrada no modo "roteiro" (sem JSPI): o que o usuário digita
    // antes/durante a execução vira um fluxo de bytes para o programa.
    this.roteiro = [];
    this.bytesRoteiro = new Uint8Array(0);

    this.xterm.onData((dados) => this._aoTeclar(dados));

    this.observador = new ResizeObserver(() => this.ajustar());
    this.observador.observe(elemento);
  }

  /** Reajusta ao tamanho do painel (chamado nas transições de layout). */
  ajustar() {
    try {
      this.ajuste.fit();
    } catch (e) { /* painel escondido: o fit reclama, mas nada quebra */ }
  }

  definirPaleta(nome) {
    this.xterm.options.theme = CORES_POR_PALETA[nome] || CORES_POR_PALETA.padrao;
  }

  escrever(texto) {
    this.xterm.write(String(texto).replace(/\n/g, '\r\n'));
  }

  escreverCru(texto) {
    this.xterm.write(texto);
  }

  limpar() {
    this.xterm.reset();
    this.xterm.clear();
  }

  _aoTeclar(dados) {
    // Repassa tudo para o modo ativo (app.js decide qual é).
    this.aoDigitar(dados);
  }

  // ------------------------------------------------------- modo interativo

  /** Pede uma linha ao usuário; devolve Promise<Uint8Array> com o Enter. */
  lerLinha() {
    return new Promise((resolver) => {
      this.aguardando = resolver;
      this.linha = '';
      this.emEdicao = true;
      // O programa está esperando: leva o foco do teclado para o terminal, para
      // que digitar funcione sem o usuário precisar clicar antes. Se ele estiver
      // editando o código, o foco volta em seguida (ver app.js).
      if (this.xterm && typeof this.xterm.focus === 'function') this.xterm.focus();
    });
  }

  /** true enquanto uma leitura está esperando o usuário. */
  esperandoEntrada() {
    return this.emEdicao;
  }

  /** Chamado pelo app.js com cada tecla (modo interativo). */
  processarTeclaInterativa(dados) {
    if (!this.emEdicao) {
      // Enter com o programa parado: melhor não fazer nada.
      return;
    }
    for (const ch of dados) {
      const codigo = ch.charCodeAt(0);
      if (ch === '\r') {
        this.xterm.write('\r\n');
        const resposta = new TextEncoder().encode(`${this.linha}\n`);
        this.linha = '';
        this.emEdicao = false;
        const resolver = this.aguardando;
        this.aguardando = null;
        if (resolver) resolver(resposta);
      } else if (ch === '\x7f' || codigo === 8) {
        if (this.linha.length > 0) {
          this.linha = this.linha.slice(0, -1);
          this.xterm.write('\b \b');
        }
      } else if (ch === '\x03') {
        this.xterm.write('^C\r\n');
        this.linha = '';
        this.emEdicao = false;
        const resolver = this.aguardando;
        this.aguardando = null;
        if (resolver) resolver(new Uint8Array(0));
      } else if (codigo >= 32) {
        this.linha += ch;
        this.xterm.write(ch);
      }
    }
  }

  // ---------------------------------------------------------- modo roteiro

  /**
   * Acrescenta texto ao roteiro de entrada.
   * `ecoar` reproduz o que foi digitado na tela (é o caso do usuário digitando);
   * entrada posta pelo programa (a de teste de um exemplo) entra calada.
   */
  adicionarAoRoteiro(dados, { ecoar = true } = {}) {
    this.roteiro.push(dados);
    this.bytesRoteiro = new TextEncoder().encode(this.roteiro.join(''));
    if (!ecoar) return;
    if (dados.includes('\r')) this.escreverCru('\r\n');
    else this.escreverCru(dados.replace(/\x7f/g, '\b \b'));
  }

  /** Consome até `n` bytes do roteiro; devolve o que houver (vazio = EOF). */
  lerRoteiro(n) {
    if (!this.bytesRoteiro.length) return new Uint8Array(0);
    const pedaco = this.bytesRoteiro.subarray(0, n);
    this.bytesRoteiro = this.bytesRoteiro.subarray(pedaco.length);
    return pedaco;
  }

  roteiroVazio() {
    return this.bytesRoteiro.length === 0;
  }

  limparRoteiro() {
    this.roteiro = [];
    this.bytesRoteiro = new Uint8Array(0);
  }

  // ---------------------------------------------------------- mensagens

  aviso(texto) { this.escreverCru(`${AZUL}${texto}${NORMAL}\r\n`); }
  sucesso(texto) { this.escreverCru(`${VERDE}${texto}${NORMAL}\r\n`); }
  erro(texto) { this.escreverCru(`${VERMELHO}${texto}${NORMAL}\r\n`); }
  detalhe(texto) { this.escreverCru(`${CINZA}${texto}${NORMAL}\r\n`); }
  destaque(texto) { this.escreverCru(`${AMARELO}${texto}${NORMAL}\r\n`); }

  centralizar() {
    this.ajustar();
  }
}

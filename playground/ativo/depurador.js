/*
 * depurador.js — o depurador opcional do playground.
 *
 * Ele **vem desligado**: com o interruptor da barra lateral apagado, nada
 * deste arquivo toca o seu programa — o fonte vai inteiro para o clang e a
 * execução é a mesma de sempre. Ligado, a página passa a fazer duas coisas:
 *
 *   1. `instrumentar()` reescreve o fonte inserindo, no começo de cada
 *      comando, uma chamada ao gancho de `tools/basicgol_depurador.h`:
 *
 *          if (bgdbg_abre(12)) { BGDBG_VAR("soma", soma); bgdbg_fecha(); } soma = soma + i;
 *
 *      A inserção é sempre no *começo* da linha, nunca em linha nova: o
 *      número de linhas não muda, então os erros do compilador continuam
 *      apontando para a linha certa do editor.
 *
 *   2. `Depurador` conversa com o programa pelo descritor 3 (ver o protocolo
 *      no cabeçalho C). A cada linha o programa manda um quadro e espera um
 *      byte; é aí que o depurador decide continuar, parar num ponto de parada
 *      ou dar um passo. A pausa de verdade — o programa congelado no meio de
 *      uma linha — depende de JSPI (`WebAssembly.Suspending`); sem JSPI o
 *      programa não pode esperar, e o depurador vira um traço de execução.
 *
 * A instrumentação é textual e conservadora: na dúvida, ela não mexe na
 * linha. O pior caso é uma linha sem ponto de parada, nunca um programa com
 * semântica diferente — e, se mesmo assim o fonte instrumentado não compilar,
 * `app.js` recompila o original e avisa.
 */

/** Comandos que a página manda ao programa (1 byte, pelo descritor 3). */
export const COMANDOS = {
  CONTINUAR: 'c',
  PASSO: 's',
  DESLIGAR: 'x',
  PARAR: 'q',
};

/** Código de saída de quem foi encerrado pelo botão Parar (= no cabeçalho C). */
export const SAIDA_PARADO = 130;

/* Teto de quadros por execução: um laço de milhões de voltas não trava a aba.
 * Cada quadro custa ~35 µs no Chrome com JSPI (medido com o laço de 10 mil
 * voltas de `tools/verificar-pagina.mjs`), então o teto vale ~2 s. */
export const LIMITE_EVENTOS = 50000;

/** Quantas linhas percorridas o histórico guarda (o painel mostra as últimas). */
const LIMITE_HISTORICO = 200;

/* ========================================================================= */
/* instrumentação                                                            */
/* ========================================================================= */

/* Palavras da linguagem que viram chave de bloco depois do pré-processador. */
const ABREM = new Set(['INICIO', 'ENTAO', 'FACA', 'REPITA', 'ESCOLHA']);
const FECHAM = new Set([
  'FIM', 'FIM_SE', 'FIM_ENQUANTO', 'FIM_ESCOLHA', 'FIM_PARA',
  'FIM_REGISTRO', 'FIM_ENUMERACAO', 'ATE_QUE', 'SENAO_SE',
]);
const REGISTROS = new Set(['REGISTRO', 'ENUMERACAO']);

/* Linhas que não recebem gancho: fecham ou continuam um bloco já aberto
 * (`SENAO` vira `} else {`, e um comando antes disso não seria C válido). */
const NAO_RECEBEM = new RegExp('^(?:[{}]|(?:#|FIM|FIM_SE|FIM_ENQUANTO|FIM_ESCOLHA'
  + '|FIM_PARA|FIM_REGISTRO|FIM_ENUMERACAO|SENAO|SENAO_SE|ATE_QUE|CASO'
  + '|CONTRARIO|else|case|default)\\b)');

/* Fim de um comando: só depois de `;`, `{`, `}`, `:` ou de uma palavra que
 * abre/fecha bloco é que a próxima linha começa um comando novo. Assim uma
 * chamada quebrada em duas linhas — ou um `if (x)` sem chaves — não leva
 * gancho no meio. */
const PALAVRAS_LIMITE = new RegExp('\\b(?:INICIO|ENTAO|FACA|REPITA|SENAO|FIM'
  + '|FIM_SE|FIM_ENQUANTO|FIM_ESCOLHA|FIM_PARA|FIM_REGISTRO|FIM_ENUMERACAO)$');

/* Cabeçalho de função: `FUNCAO REAL media(REAL a, REAL b) {`, `int main(void) {`. */
const CABECALHO_FUNCAO = /^[A-Za-z_][\w \t*]*\([^;]*\)\s*$/;

/* Tipos que começam uma declaração (os longos antes dos curtos). */
const TIPOS = [
  'INTEIRO_CURTO', 'INTEIRO_LONGO', 'INTEIRO8', 'INTEIRO16', 'INTEIRO32', 'INTEIRO64',
  'INTEIRO', 'NATURAL_CURTO', 'NATURAL_LONGO', 'NATURAL8', 'NATURAL16', 'NATURAL32',
  'NATURAL64', 'NATURAL', 'REAL_CURTO', 'REAL_LONGO', 'REAL', 'LOGICO', 'CARACTERE',
  'TEXTO', 'long double', 'unsigned long long', 'unsigned long', 'unsigned int',
  'unsigned char', 'unsigned short', 'unsigned', 'long long', 'long', 'int', 'short',
  'char', 'float', 'double', 'size_t', 'bool', '_Bool',
].join('|');
const PREFIXOS = '(?:CONSTANTE|ESTATICO|EXTERNO|static|const|register|volatile)';
const DECLARACAO = new RegExp(`^(?:${PREFIXOS}\\s+)*(${TIPOS})\\s+([^;]+);`);
const TEXTO_FIXO = /^TEXTO_FIXO\s*\(\s*([A-Za-z_]\w*)\s*,/;
/* Tipos cujo valor é lido pelo ponteiro: só mostramos depois de preenchidos. */
const TIPOS_TEXTO = new Set(['TEXTO', 'char', 'TEXTO_FIXO']);

const escapar = (nome) => nome.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');

/**
 * Devolve a linha sem comentários nem literais — é sobre ela que as decisões
 * são tomadas (um `;` dentro de "texto" não termina comando nenhum).
 * `estado.comentario` atravessa as linhas (comentário de bloco).
 */
function limpar(linha, estado) {
  let saida = '';
  let i = 0;
  while (i < linha.length) {
    if (estado.comentario) {
      const fim = linha.indexOf('*/', i);
      if (fim < 0) return saida;
      estado.comentario = false;
      i = fim + 2;
      continue;
    }
    const c = linha[i];
    const proximo = linha[i + 1];
    if (c === '/' && proximo === '/') break;
    if (c === '/' && proximo === '*') { estado.comentario = true; i += 2; continue; }
    if (c === '"' || c === "'") {
      saida += c === '"' ? '""' : "''"; // o literal vira um vazio do mesmo tipo
      i += 1;
      while (i < linha.length) {
        if (linha[i] === '\\') { i += 2; continue; }
        if (linha[i] === c) { i += 1; break; }
        i += 1;
      }
      continue;
    }
    saida += c;
    i += 1;
  }
  return saida;
}

/** Separa `a = 1, b = 2` em declaradores, ignorando vírgulas aninhadas. */
function declaradores(texto) {
  const partes = [];
  let nivel = 0;
  let atual = '';
  for (const c of texto) {
    if (c === '(' || c === '[') nivel += 1;
    else if (c === ')' || c === ']') nivel -= 1;
    if (c === ',' && nivel === 0) { partes.push(atual); atual = ''; continue; }
    atual += c;
  }
  partes.push(atual);
  return partes;
}

/**
 * Variáveis declaradas nesta linha (só as que o depurador sabe mostrar:
 * escalares e texto; vetores, ponteiros e registros ficam de fora).
 */
function declaracoesDe(limpa) {
  const achadas = [];
  const fixo = TEXTO_FIXO.exec(limpa.trim());
  if (fixo) {
    achadas.push({ nome: fixo[1], texto: true, pronta: false });
    return achadas;
  }
  const m = DECLARACAO.exec(limpa.trim());
  if (!m) return achadas;
  const texto = TIPOS_TEXTO.has(m[1]);
  for (const parte of declaradores(m[2])) {
    const d = /^\s*([A-Za-z_]\w*)\s*(.*)$/.exec(parte);
    if (!d) continue;
    const resto = d[2].trim();
    if (parte.includes('*') || resto.startsWith('[') || resto.startsWith('(')) continue;
    achadas.push({ nome: d[1], texto, pronta: !texto || resto.startsWith('=') });
  }
  return achadas;
}

/** Parâmetros de um cabeçalho de função (sem vetores nem ponteiros). */
function parametrosDe(limpa) {
  const abre = limpa.indexOf('(');
  const fecha = limpa.lastIndexOf(')');
  if (abre < 0 || fecha < abre) return [];
  const dentro = limpa.slice(abre + 1, fecha).trim();
  if (!dentro || dentro === 'void' || dentro === 'VAZIO') return [];
  const achados = [];
  for (const parte of declaradores(dentro)) {
    const m = new RegExp(`^\\s*(?:${PREFIXOS}\\s+)*(${TIPOS})\\s+([A-Za-z_]\\w*)\\s*$`)
      .exec(parte.trim());
    if (!m) continue;
    achados.push({ nome: m[2], texto: TIPOS_TEXTO.has(m[1]), pronta: true });
  }
  return achados;
}

/** Marca como preenchidas as variáveis de texto atribuídas nesta linha. */
function marcarAtribuicoes(limpa, visiveis) {
  for (const v of visiveis) {
    if (v.pronta) continue;
    const n = escapar(v.nome);
    const padrao = new RegExp(`\\b${n}\\s*(?:\\[[^\\]]*\\])?\\s*=(?!=)`
      + `|\\bLEIA(?:_TEXTO|_INTEIRO|_REAL|_OK)?\\s*\\(\\s*${n}\\b`
      + `|\\b(?:COPIA_TEXTO|JUNTA_TEXTO|ACRESCENTA_TEXTO|TRECHO_TEXTO|FORMATA_TEXTO)`
      + `\\s*\\(\\s*${n}\\b`
      + `|\\bPARA(?:_PASSO)?\\s*\\(\\s*${n}\\b`);
    if (padrao.test(limpa)) v.pronta = true;
  }
}

/** Está dentro de um corpo de função (e não dentro de um REGISTRO)? */
function dentroDeFuncao(pilha) {
  if (!pilha.length || pilha[0].tipo !== 'funcao') return false;
  return !pilha.some((e) => e.tipo === 'registro');
}

/** A linha anterior terminou um comando? */
function limiteDeComando(anterior) {
  if (anterior === null) return true;
  const t = anterior.trim();
  if (t === '') return true;
  if (/[;{}:]$/.test(t)) return true;
  return PALAVRAS_LIMITE.test(t);
}

/**
 * Insere os ganchos do depurador no fonte.
 *
 * @param {string} fonte                 o programa já preparado para o clang
 * @param {object} [opcoes]
 * @param {number} [opcoes.maxVariaveis] variáveis por quadro (as mais internas)
 * @returns {{fonte: string, linhas: number[]}} fonte instrumentado e as
 *   linhas (1-based, no mesmo arquivo) onde um ponto de parada funciona.
 */
export function instrumentar(fonte, { maxVariaveis = 16 } = {}) {
  const linhas = String(fonte).split('\n');
  const estado = { comentario: false };
  const pilha = [];
  const instrumentadas = [];
  const saida = [];
  let anterior = null;
  let emInicializador = false;

  const visiveis = () => {
    const mapa = new Map();
    for (const escopo of pilha) {
      for (const v of escopo.nomes) mapa.set(v.nome, v);
    }
    return [...mapa.values()];
  };

  for (let i = 0; i < linhas.length; i += 1) {
    const original = linhas[i];
    const limpa = limpar(original, estado).trim();
    const numero = i + 1;

    if (emInicializador) {
      // Dentro de `= { ... }` não há comandos nem chaves de bloco.
      if (/\}\s*;?\s*$/.test(limpa)) emInicializador = false;
      saida.push(original);
      if (limpa) anterior = limpa;
      continue;
    }

    const lista = visiveis();
    const podeInstrumentar = limpa !== ''
      && dentroDeFuncao(pilha)
      && limiteDeComando(anterior)
      && !NAO_RECEBEM.test(limpa)
      && !/^[A-Za-z_]\w*\s*:$/.test(limpa); // rótulo (CASO já sai no NAO_RECEBEM)

    if (podeInstrumentar) {
      const mostradas = lista.filter((v) => !v.texto || v.pronta).slice(-maxVariaveis);
      const campos = mostradas.map((v) => `BGDBG_VAR("${v.nome}", ${v.nome});`).join(' ');
      const gancho = `if (bgdbg_abre(${numero})) { `
        + `${campos}${campos ? ' ' : ''}bgdbg_fecha(); } `;
      const recuo = original.match(/^[ \t]*/)[0];
      saida.push(recuo + gancho + original.slice(recuo.length));
      instrumentadas.push(numero);
    } else {
      saida.push(original);
    }

    // --- estado para as próximas linhas
    marcarAtribuicoes(limpa, lista);

    const cabecalho = pilha.length === 0
      && (CABECALHO_FUNCAO.test(limpa.replace(/\{$/, '').trim()) && limpa.endsWith('{'));
    const cabecalhoAnterior = pilha.length === 0 && limpa === '{'
      && anterior !== null && CABECALHO_FUNCAO.test(anterior.trim());

    if (cabecalho || cabecalhoAnterior) {
      const fonteParametros = cabecalho ? limpa : anterior;
      pilha.push({ tipo: 'funcao', nomes: parametrosDe(fonteParametros) });
    } else {
      for (const token of limpa.match(/[A-Za-z_]\w*|[{}]/g) || []) {
        if (token === '{') pilha.push({ tipo: 'bloco', nomes: [] });
        else if (token === '}') pilha.pop();
        else if (token === 'SENAO') { pilha.pop(); pilha.push({ tipo: 'bloco', nomes: [] }); }
        else if (FECHAM.has(token)) pilha.pop();
        else if (REGISTROS.has(token)) pilha.push({ tipo: 'registro', nomes: [] });
        else if (token === 'INICIO') pilha.push({ tipo: 'funcao', nomes: [] });
        else if (ABREM.has(token)) pilha.push({ tipo: 'bloco', nomes: [] });
      }
      if (pilha.length && dentroDeFuncao(pilha)) {
        pilha[pilha.length - 1].nomes.push(...declaracoesDe(limpa));
      }
    }

    if (/=\s*\{[^}]*$/.test(limpa)) emInicializador = true;
    if (limpa) anterior = limpa;
  }

  return { fonte: saida.join('\n'), linhas: instrumentadas };
}

/* ========================================================================= */
/* sessão                                                                    */
/* ========================================================================= */

const codificador = new TextEncoder();
const decodificador = new TextDecoder();
const byte = (comando) => codificador.encode(comando);

/**
 * O lado da página: lê os quadros do programa e decide parar ou seguir.
 *
 * Não conhece o DOM — `app.js` desenha a partir de `aoMudar`, e
 * `tools/verificar-depurador.mjs` roda a mesma máquina fora do navegador.
 */
export class Depurador {
  constructor({ aoMudar = () => {}, limite = LIMITE_EVENTOS } = {}) {
    this.aoMudar = aoMudar;
    this.limite = limite;

    this.ativo = false; // o interruptor da barra lateral (desligado por padrão)
    this.pontos = new Set(); // linhas do editor com ponto de parada
    this.linhasValidas = []; // linhas instrumentadas na última compilação

    this.estado = 'ocioso'; // 'ocioso' | 'executando' | 'parado' | 'encerrado'
    this.linha = 0;
    this.variaveis = [];
    this.historico = [];
    this.eventos = 0;
    this.aviso = '';

    this.deslocamento = 0;
    this.podePausar = false;
    this.pedidoParar = false; // o botão Parar já foi apertado nesta execução

    this._buffer = '';
    this._quadro = null;
    this._pendente = null;
    this._resolver = null;
    this._passo = false;
    this._jaParou = false;
  }

  /** Liga/desliga o depurador (o interruptor da barra lateral). */
  definirAtivo(ativo) {
    this.ativo = !!ativo;
    if (!this.ativo) {
      this.estado = 'ocioso';
      this.linha = 0;
      this.variaveis = [];
    }
    this.aoMudar(this);
  }

  alternarPonto(linha) {
    if (this.pontos.has(linha)) this.pontos.delete(linha);
    else this.pontos.add(linha);
    this.aoMudar(this);
    return this.pontos.has(linha);
  }

  limparPontos() {
    this.pontos.clear();
    this.aoMudar(this);
  }

  /** Prepara uma execução. `deslocamento` = linhas somadas antes do programa. */
  iniciar({ deslocamento = 0, podePausar = false, linhasValidas = null } = {}) {
    this.deslocamento = deslocamento;
    this.podePausar = podePausar;
    if (linhasValidas) this.linhasValidas = linhasValidas;
    this.estado = 'executando';
    this.linha = 0;
    this.variaveis = [];
    this.historico = [];
    this.eventos = 0;
    this.aviso = '';
    this._buffer = '';
    this._quadro = null;
    this._pendente = null;
    this._resolver = null;
    this._passo = false;
    this.pedidoParar = false;
    // Sem ponto de parada nenhum, uma execução com depurador para na primeira
    // linha — senão ligar o depurador não mudaria nada visível.
    this._jaParou = this.pontos.size > 0;
    this.aoMudar(this);
  }

  /** O canal do descritor 3, como `armazenamento.js` espera. */
  canal() {
    return {
      escrever: (bytes) => this._receber(bytes),
      ler: () => this._entregar(),
    };
  }

  /** true quando o programa está congelado esperando um comando. */
  estaParado() {
    return this.estado === 'parado';
  }

  continuar() {
    this._passo = false;
    this._responder(COMANDOS.CONTINUAR);
  }

  passo() {
    this._passo = true;
    this._responder(COMANDOS.PASSO);
  }

  /** Botão Parar: encerra o programa na próxima linha (ou agora, se parado). */
  parar() {
    this.pedidoParar = true;
    this._passo = false;
    this._responder(COMANDOS.PARAR);
  }

  /** Fim da execução: solta o que estiver pendente e volta ao repouso. */
  encerrar() {
    if (this._resolver) {
      const resolver = this._resolver;
      this._resolver = null;
      resolver(byte(COMANDOS.DESLIGAR));
    }
    this.estado = 'encerrado';
    this.linha = 0;
    this.aoMudar(this);
  }

  /* ------------------------------------------------------------- interno */

  _responder(comando) {
    if (!this._resolver) return;
    const resolver = this._resolver;
    this._resolver = null;
    this.estado = 'executando';
    this.aoMudar(this);
    resolver(byte(comando));
  }

  /** Recebe bytes do programa e monta os quadros linha a linha. */
  _receber(bytes) {
    this._buffer += decodificador.decode(bytes, { stream: true });
    let quebra = this._buffer.indexOf('\n');
    while (quebra >= 0) {
      const linha = this._buffer.slice(0, quebra);
      this._buffer = this._buffer.slice(quebra + 1);
      this._processarLinha(linha);
      quebra = this._buffer.indexOf('\n');
    }
  }

  _processarLinha(linha) {
    if (linha.startsWith('l ')) {
      this._quadro = { linha: Number(linha.slice(2)) || 0, variaveis: [] };
      return;
    }
    if (linha.startsWith('v ') && this._quadro) {
      const m = /^v (\S+) (\S+) ?([\s\S]*)$/.exec(linha);
      if (m) this._quadro.variaveis.push({ nome: m[1], tipo: m[2], valor: m[3] });
      return;
    }
    if (linha === 'e' && this._quadro) {
      this._pendente = this._decidir(this._quadro);
      this._quadro = null;
    }
  }

  /** O que responder a este quadro: continuar, desligar, encerrar ou parar. */
  _decidir(quadro) {
    this.eventos += 1;
    this.linha = Math.max(1, quadro.linha - this.deslocamento);
    this.variaveis = quadro.variaveis;
    this.historico.push({ linha: this.linha, variaveis: quadro.variaveis });
    if (this.historico.length > LIMITE_HISTORICO) this.historico.shift();

    if (this.pedidoParar) return byte(COMANDOS.PARAR);
    if (this.eventos > this.limite) {
      this.aviso = `limite de ${this.limite} linhas depuradas atingido: `
        + 'o programa segue sem o depurador.';
      this.estado = 'executando';
      this.aoMudar(this);
      return byte(COMANDOS.DESLIGAR);
    }
    if (!this.podePausar) return byte(COMANDOS.CONTINUAR); // modo traço

    const parar = this._passo || this.pontos.has(this.linha) || !this._jaParou;
    if (!parar) return byte(COMANDOS.CONTINUAR);

    this._jaParou = true;
    this.estado = 'parado';
    this.aoMudar(this);
    return new Promise((resolver) => { this._resolver = resolver; });
  }

  /** A resposta que o `fd_read` do descritor 3 devolve (byte ou Promise). */
  _entregar() {
    const resposta = this._pendente;
    this._pendente = null;
    return resposta || byte(COMANDOS.CONTINUAR);
  }
}

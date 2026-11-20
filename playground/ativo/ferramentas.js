/*
 * ferramentas.js — compila e liga BASICGOL **dentro do navegador**.
 *
 * Emscripten + WebAssembly de verdade, sem servidor:
 *
 *   1. O playground baixa `publico/toolchain/wasi-cc.tar.gz`, que traz clang e
 *      wasm-ld compilados para WebAssembly (o projeto binji/wasm-clang), o
 *      sysroot da wasi-libc e o cabeçalho + a biblioteca padrão do BASICGOL.
 *   2. clang e wasm-ld rodam na página. O "disco" deles é o módulo `memfs`:
 *      o host (esta página) responde `path_open`, `fd_read`, ... com imports
 *      JavaScript. É nele que ficam o seu código, `include/basicgol.h` e
 *      `src/biblioteca/*.c`.
 *   3. O clang gera um `.o` por arquivo; o wasm-ld liga tudo com a libc e
 *      produz `programa.wasm`, que o terminal executa.
 *
 * Depois do primeiro download, compilar e rodar é 100% local — nada do que
 * você escreve sai do navegador.
 *
 * O protocolo abaixo é o mesmo que `tools/verificar.js` usa no Node: aquele
 * script é a prova de fumaça de que estes caminhos e argumentos estão certos.
 */

import { criarImportsMemfs } from './armazenamento.js';

const CAMINHO_PACOTE = '../publico/toolchain/wasi-cc.tar.gz';
const CODIFICADOR = new TextEncoder();
const DECODIFICADOR = new TextDecoder();

/** Diretórios de cabeçalhos do sysroot (os do binji/wasm-clang). */
const CABECALHOS = [
  'usr/include/c++/v1',
  'usr/include',
  'usr/lib/clang/8.0.1/include',
];
const LIB = 'usr/lib/wasm32-wasi';
const BUILTINS = 'usr/lib/clang/8.0.1/lib/wasi/libclang_rt.builtins-wasm32.a';

/* ------------------------------------------------------------------------- */
/* tar e zip                                                                  */
/* ------------------------------------------------------------------------- */

/**
 * Percorre um tar ustar chamando `aoEntrada(tipo, nome, bytes)`.
 * Tipo '5' = diretório, '0' (ou vazio) = arquivo.
 */
export function percorrerTar(bytes, aoEntrada) {
  const u8 = bytes instanceof Uint8Array ? bytes : new Uint8Array(bytes);
  const texto = (o, n) => DECODIFICADOR.decode(u8.subarray(o, o + n)).replace(/\0[\s\S]*$/, '');
  let o = 0;
  while (o + 512 <= u8.length) {
    const nome = texto(o, 100);
    if (!nome) break;
    const tamanho = parseInt(texto(o + 124, 12).trim() || '0', 8) || 0;
    const tipo = String.fromCharCode(u8[o + 156] || 48);
    const prefixo = texto(o + 345, 155);
    const caminho = (prefixo ? `${prefixo}/${nome}` : nome).replace(/\/$/, '');
    aoEntrada(tipo, caminho, u8.subarray(o + 512, o + 512 + tamanho));
    o += 512 + Math.ceil(tamanho / 512) * 512;
  }
}

/** Descomprime gzip/deflate com a API do navegador (nada de libs externas). */
export async function descomprimir(bytes, formato = 'gzip') {
  if (typeof DecompressionStream === 'undefined') {
    throw new Error('este navegador não descompacta gzip (sem DecompressionStream)');
  }
  const fluxo = new Blob([bytes]).stream().pipeThrough(new DecompressionStream(formato));
  return new Uint8Array(await new Response(fluxo).arrayBuffer());
}

/** Descompacta um .zip (métodos 0 e 8) e devolve [{nome, bytes}]. */
export async function lerZip(bytes) {
  const u8 = bytes instanceof Uint8Array ? bytes : new Uint8Array(bytes);
  const dv = new DataView(u8.buffer, u8.byteOffset, u8.byteLength);
  let fim = -1;
  for (let i = u8.length - 22; i >= Math.max(0, u8.length - 66560); i -= 1) {
    if (dv.getUint32(i, true) === 0x06054b50) { fim = i; break; }
  }
  if (fim < 0) throw new Error('arquivo .zip inválido (não achei o diretório central)');
  const total = dv.getUint16(fim + 10, true);
  let p = dv.getUint32(fim + 16, true);
  const arquivos = [];
  for (let i = 0; i < total; i += 1) {
    if (dv.getUint32(p, true) !== 0x02014b50) break;
    const metodo = dv.getUint16(p + 10, true);
    const comprimido = dv.getUint32(p + 20, true);
    const tamanho = dv.getUint32(p + 24, true);
    const nLen = dv.getUint16(p + 28, true);
    const eLen = dv.getUint16(p + 30, true);
    const cLen = dv.getUint16(p + 32, true);
    const local = dv.getUint32(p + 42, true);
    const nome = DECODIFICADOR.decode(u8.subarray(p + 46, p + 46 + nLen));
    p += 46 + nLen + eLen + cLen;
    if (nome.endsWith('/')) continue;
    const inicio = local + 30 + dv.getUint16(local + 26, true) + dv.getUint16(local + 28, true);
    const dados = u8.subarray(inicio, inicio + comprimido);
    let conteudo;
    if (metodo === 0) {
      conteudo = dados.slice();
    } else if (metodo === 8) {
      const saida = await descomprimir(dados, 'deflate-raw');
      conteudo = saida.length > tamanho ? saida.subarray(0, tamanho) : saida;
    } else {
      throw new Error(`método de compressão ${metodo} não suportado no .zip`);
    }
    arquivos.push({ nome: nome.replace(/^\.?\//, ''), bytes: conteudo });
  }
  return arquivos;
}

/* ------------------------------------------------------------------------- */

/**
 * Dicas para os erros mais comuns de quem está aprendendo (a tabela de
 * `docs/01-especificacao.md` §12.4 lista o que o clang diz para cada um).
 * A mensagem do compilador continua sendo a principal: a dica só a traduz.
 */
const DICAS = [
  {
    quando: (codigo, log) => /expected ';' after expression/.test(log),
    dica: 'faltou ponto e vírgula no fim do comando (o ^ mostra onde).',
  },
  {
    quando: (codigo, log) => /expected declaration or statement at end of input/.test(log),
    dica: 'sobrou um bloco aberto: todo SE/ENQUANTO/REPITA/INICIO precisa do seu FIM.',
  },
  {
    quando: (codigo, log) => /^[ \t]*PARA\s+\w+\s+DE\b/m.test(codigo) && /BG_PARA|expected/.test(log),
    dica: 'o laco com DE/ATE/FACA existe so no caminho M1. No modo macro: PARA (i, 1, 10) { ... }',
  },
  {
    quando: (codigo) => /^[ \t]*SE\b[^{\n]*\{/m.test(codigo),
    dica: 'SE já abre parêntese: escreva SE cond ENTAO (sem { e sem ;).',
  },
  {
    quando: (codigo) => /^[ \t]*SENAO\s+SE\b/m.test(codigo),
    dica: 'SENAO SE não encadeia: use SENAO_SE.',
  },
  {
    quando: (codigo) => /^[ \t]*VETOR\s+(INTEIRO|REAL|TEXTO|CARACTERE|LOGICO)\b/m.test(codigo),
    dica: 'VETOR é uma chamada: VETOR(INTEIRO, v, 10);',
  },
  {
    quando: (codigo) => /MOD\s*=/.test(codigo),
    dica: 'não existe MOD=: use x = x MOD 3;',
  },
];

/** Dicas que se aplicam a este programa, dado o log do compilador. */
export function dicasPara(codigo, log) {
  const texto = String(codigo || '');
  return DICAS.filter((d) => d.quando(texto, log)).map((d) => d.dica);
}

export class Ferramentas {
  constructor({ aoProgredir = () => {}, aoTraco = null } = {}) {
    this.aoProgredir = aoProgredir;
    this.aoTraco = aoTraco; // linhas de diagnóstico (console, testes)
    this.modulos = new Map(); // nome -> WebAssembly.Module
    this.pacote = null; // Uint8Array do wasi-cc.tar.gz já descomprimido
    this.memfs = null; // { exports, memoria, host }
    this.arquivosUsuario = [];
    this.carregando = null;
    this.pronto = false;
    this.aoSaida = null; // coletor da saída do clang/lld
    this._fontesBiblioteca = null;
  }

  /** Baixa e prepara as ferramentas (uma vez; chamadas concorrentes se juntam). */
  carregar() {
    if (!this.carregando) {
      this.carregando = this._carregar().catch((e) => {
        this.carregando = null; // permite tentar de novo
        throw e;
      });
    }
    return this.carregando;
  }

  async _carregar() {
    const inicio = performance.now();
    this.aoProgredir({ etapa: 'download', mensagem: 'baixando clang, wasm-ld e a libc (18 MB)' });

    const resposta = await fetch(new URL(CAMINHO_PACOTE, import.meta.url), { cache: 'force-cache' });
    if (!resposta.ok) {
      throw new Error(`falha ao baixar o pacote de ferramentas (HTTP ${resposta.status}). `
        + 'Gere-o com "node playground/tools/empacotar-toolchain.js".');
    }
    // O pacote é um .tar.gz: descomprimimos na própria página.
    const comprimido = new Uint8Array(await resposta.arrayBuffer());
    this.pacote = comprimido[0] === 0x1f && comprimido[1] === 0x8b
      ? await descomprimir(comprimido, 'gzip')
      : comprimido;

    this.aoProgredir({ etapa: 'preparar', mensagem: 'preparando clang e wasm-ld' });
    for (const nome of ['clang', 'lld', 'memfs']) {
      this.modulos.set(nome, await WebAssembly.compile(this._extrairDoPacote(nome)));
    }

    this.aoProgredir({ etapa: 'iniciar', mensagem: 'iniciando o disco do compilador' });
    const instancia = await WebAssembly.instantiate(this.modulos.get('memfs'), {
      env: criarImportsMemfs({
        obterMemoria: () => this.memfs && this.memfs.host,
        obterMemfs: () => this.memfs,
        aoEscrever: (_fd, texto) => { if (this.aoSaida !== null) this.aoSaida += texto; },
        aoLogar: (texto) => { if (this.aoTraco) this.aoTraco(`memfs: ${texto.trim()}`); },
      }),
    });
    this.memfs = { exports: instancia.exports, memoria: instancia.exports.memory, host: null };
    instancia.exports.init();

    this.aoProgredir({ etapa: 'montar', mensagem: 'copiando o sysroot para a memória' });
    let arquivos = 0;
    percorrerTar(this._extrairDoPacote('sysroot.tar'), (tipo, caminho, dados) => {
      if (tipo === '5') {
        this._pasta(`usr/${caminho}`);
      } else {
        this._arquivo(`usr/${caminho}`, dados);
        arquivos += 1;
      }
    });
    if (this.aoTraco) {
      this.aoTraco(`sysroot: ${arquivos} arquivos, `
        + `${this.memfs.memoria.buffer.byteLength / 65536} páginas`);
    }

    this.pronto = true;
    this.aoProgredir({
      etapa: 'pronto',
      mensagem: `ferramentas prontas em ${((performance.now() - inicio) / 1000).toFixed(1)}s`,
    });
  }

  /** Extrai um membro do pacote (`clang`, `sysroot.tar`, `include/basicgol.h`, ...). */
  _extrairDoPacote(nome) {
    let achado = null;
    percorrerTar(this.pacote, (tipo, caminho, dados) => {
      if (tipo !== '5' && caminho === nome) achado = dados;
    });
    if (!achado) throw new Error(`pacote incompleto: falta "${nome}"`);
    return achado;
  }

  /** Lista os nomes de arquivo do pacote sob `prefixo`, relativos a ele. */
  _listarPacote(prefixo) {
    const nomes = [];
    percorrerTar(this.pacote, (tipo, caminho) => {
      if (tipo !== '5' && caminho.startsWith(prefixo)) nomes.push(caminho.slice(prefixo.length));
    });
    return nomes.sort();
  }

  /* ----------------------------------------------------- memfs: escrita --- */

  /**
   * Existe algum nó (arquivo ou diretório) com este caminho?
   *
   * O memfs não tem "existe?": `FindNode` **aborta** quando o caminho não
   * existe (é um ASSERT, não um retorno de erro). O abort é um `unreachable`
   * do wasm, que o host recebe como exceção e que deixa a instância intacta —
   * então a trap *é* a resposta. (`path_filestat_get` não serve daqui: ele lê o
   * caminho da memória do host, que só existe durante clang/lld.)
   */
  _existe(caminho) {
    const m = this.memfs.exports;
    const nome = CODIFICADOR.encode(caminho);
    new Uint8Array(this.memfs.memoria.buffer, m.GetPathBuf(), nome.length).set(nome);
    try {
      return m.FindNode(nome.length) !== 0xffffffff;
    } catch (e) {
      return false;
    }
  }

  /**
   * Cria os diretórios de `caminho` e os pais que faltarem.
   *
   * O memfs não tem "criar se não existir": `AddDirectoryNode` num caminho já
   * existente cria um diretório paralelo e queima um nó da tabela. Por isso a
   * checagem antes de cada criação — sem ela, recompilar várias vezes esvazia
   * a tabela de nós do compilador.
   */
  _pasta(caminho) {
    const m = this.memfs.exports;
    const partes = String(caminho).split('/').filter(Boolean);
    for (let i = 1; i <= partes.length; i += 1) {
      const parcial = partes.slice(0, i).join('/');
      if (this._existe(parcial)) continue;
      const bytes = CODIFICADOR.encode(parcial);
      new Uint8Array(this.memfs.memoria.buffer, m.GetPathBuf(), bytes.length).set(bytes);
      m.AddDirectoryNode(bytes.length);
    }
  }

  /**
   * Escreve um arquivo no memfs (criando os diretórios).
   *
   * `AddFileNode` num caminho existente reaproveita o nó e ajusta o tamanho;
   * os bytes novos são escritos por cima. É isso que permite recompilar no
   * mesmo disco sem estourar a tabela nem vazar memória.
   */
  _arquivo(caminho, dados) {
    const m = this.memfs.exports;
    const bytes = typeof dados === 'string' ? CODIFICADOR.encode(dados) : dados;
    const partes = caminho.split('/');
    partes.pop();
    if (partes.length) this._pasta(partes.join('/'));
    const nome = CODIFICADOR.encode(caminho);
    new Uint8Array(this.memfs.memoria.buffer, m.GetPathBuf(), nome.length).set(nome);
    const inode = m.AddFileNode(nome.length, bytes.length);
    if (inode < 0) throw new Error(`o disco do compilador encheu (${caminho})`);
    new Uint8Array(this.memfs.memoria.buffer, m.GetFileNodeAddress(inode), bytes.length).set(bytes);
    return inode;
  }

  /** Lê um arquivo do memfs (ou null se não existir). */
  _lerArquivo(caminho) {
    const m = this.memfs.exports;
    if (!this._existe(caminho)) return null;
    const nome = CODIFICADOR.encode(caminho);
    new Uint8Array(this.memfs.memoria.buffer, m.GetPathBuf(), nome.length).set(nome);
    const inode = m.FindNode(nome.length);
    const inicio = m.GetFileNodeAddress(inode);
    return new Uint8Array(this.memfs.memoria.buffer, inicio, m.GetFileNodeSize(inode)).slice();
  }

  /**
   * Copia para o disco do compilador o que ele abre nesta compilação: o
   * programa, o shim, o cabeçalho de compatibilidade, `<basicgol.h>`, a
   * biblioteca padrão e os arquivos enviados pelo usuário.
   */
  _montarDisco(codigo, arquivosUsuario, depurar = false) {
    // `-Iinclude` faz `#include <basicgol/...>` procurar em include/basicgol.
    this._arquivo('main.cc', codigo);
    // O depurador é opcional: o cabeçalho só vai para o disco (e só é
    // injetado, mais abaixo, no arquivo do usuário) quando está ligado.
    if (depurar) {
      this._arquivo('basicgol_depurador.h', this._extrairDoPacote('basicgol_depurador.h'));
    }
    this._arquivo('include/basicgol.h', this._extrairDoPacote('include/basicgol.h'));
    for (const nome of this._listarPacote('include/basicgol/')) {
      this._arquivo(`include/basicgol/${nome}`, this._extrairDoPacote(`include/basicgol/${nome}`));
    }
    // `-include basicgol_playground.h` procura no diretório corrente.
    this._arquivo('basicgol_playground.h', this._extrairDoPacote('basicgol_playground.h'));
    // `src/shim.c` fica onde os caminhos do repositório esperam encontrá-lo.
    this._arquivo('src/shim.c', this._extrairDoPacote('src/shim.c'));
    if (!this._fontesBiblioteca) {
      this._fontesBiblioteca = this._listarPacote('src/biblioteca/').filter((f) => f.endsWith('.c'));
    }
    for (const nome of this._fontesBiblioteca) {
      this._arquivo(`src/biblioteca/${nome}`, this._extrairDoPacote(`src/biblioteca/${nome}`));
    }
    for (const { nome, bytes } of arquivosUsuario || []) {
      this._arquivo(nome, bytes);
    }
    return this._fontesBiblioteca;
  }

  /* ------------------------------------------------------- execução ------- */

  /**
   * Roda clang ou wasm-ld com o memfs como WASI (é assim que o binji/wasm-clang
   * funciona: o módulo exporta os handlers WASI e o host os chama direto).
   */
  async _executar(nome, argumentos) {
    const lista = [nome === 'lld' ? 'wasm-ld' : nome, ...argumentos];
    const tamanhoLista = lista.reduce((n, s) => n + CODIFICADOR.encode(s).length + 1, 0);
    let memoria = null;
    const dv = () => new DataView(memoria.buffer);
    const u8 = (o, n) => new Uint8Array(memoria.buffer, o, n);

    this.aoSaida = '';
    const wasi = {
      proc_exit: (codigo) => {
        const e = new Error('proc_exit');
        e.codigoSaida = codigo;
        throw e;
      },
      environ_sizes_get: (a, b) => { dv().setUint32(a, 0, true); dv().setUint32(b, 0, true); return 0; },
      environ_get: () => 0,
      args_sizes_get: (q, b) => {
        dv().setUint32(q, lista.length, true);
        dv().setUint32(b, tamanhoLista, true);
        return 0;
      },
      args_get: (argv, buf) => {
        let p = buf;
        lista.forEach((s, i) => {
          const b = CODIFICADOR.encode(s);
          dv().setUint32(argv + i * 4, p, true);
          u8(p, b.length).set(b);
          u8(p + b.length, 1)[0] = 0;
          p += b.length + 1;
        });
        return 0;
      },
      random_get: (ptr, tam) => {
        const destino = u8(ptr, tam);
        if (globalThis.crypto && globalThis.crypto.getRandomValues) globalThis.crypto.getRandomValues(destino);
        else for (let i = 0; i < tam; i += 1) destino[i] = (Math.random() * 256) | 0;
        return 0;
      },
      clock_time_get: (_id, _precisao, ptr) => {
        dv().setBigUint64(ptr, BigInt(Date.now()) * 1000000n, true);
        return 0;
      },
      poll_oneoff: () => 0,
    };

    const instancia = await WebAssembly.instantiate(this.modulos.get(nome), {
      wasi_unstable: { ...wasi, ...this.memfs.exports },
      env: {},
    });
    memoria = instancia.exports.memory;
    this.memfs.host = memoria;

    let codigo = 0;
    try {
      instancia.exports._start();
    } catch (e) {
      if (e.codigoSaida === undefined) throw e;
      codigo = e.codigoSaida;
    }
    const saida = this.aoSaida;
    this.aoSaida = null;
    return { codigo, saida };
  }

  /**
   * Compila e liga o programa.
   *
   * @param {object} o
   * @param {string} o.codigo                   fonte do usuário (BASICGOL/C)
   * @param {string[]} [o.flags]                flags extras (-Wall, -Werror, ...)
   * @param {string} [o.otimizacao]             -O0 .. -O3
   * @param {Array<{nome:string,bytes:Uint8Array}>} [o.arquivos]
   * @param {boolean} [o.depurar]               injeta o cabeçalho do depurador
   * @param {(linha:string)=>void} [o.aoRegistrar] recebe o log do compilador
   * @returns {Promise<{sucesso:boolean, wasm?:Uint8Array, registro:string}>}
   */
  async compilar({
    codigo, flags = [], otimizacao = '-O2', arquivos = null,
    deslocamentoLinhas = 0,
    depurar = false,
    aoGerarC = null,
    aoRegistrar = () => {},
  }) {
    await this.carregar();
    const registro = [];
    /**
     * O programa vai para o compilador num arquivo `main.cc` com algumas linhas
     * acrescentadas no topo (o `#include <basicgol.h>` que o playground põe
     * sozinho). O log do clang cita `main.cc:<linha>`, então descontamos essas
     * linhas para o número bater com o que está no editor.
     */
    const ajustarLinhas = (texto) => {
      if (!deslocamentoLinhas || !texto) return texto;
      return texto.replace(/main\.cc:(\d+)/g, (tudo, numero) => {
        const original = Number(numero) - deslocamentoLinhas;
        return original >= 1 ? `main.cc:${original}` : tudo;
      });
    };
    const registrar = (linha) => {
      if (!linha) return;
      const ajustada = ajustarLinhas(linha);
      registro.push(ajustada);
      aoRegistrar(ajustada);
    };

    const fontesBiblioteca = this._montarDisco(
      codigo, arquivos || this.arquivosUsuario, depurar,
    );

    const base = [
      '-cc1', '-disable-free',
      '-isysroot', '',
      ...CABECALHOS.flatMap((d) => ['-internal-isystem', d]),
      '-ferror-limit', '19',
      '-fmessage-length', '0',
      '-fno-color-diagnostics',
      '-mthread-model', 'single',
      '-std=c11',
      otimizacao,
      '-Iinclude',
      // Declara `mkstemp` (a libc do playground não tem; ver tools/shim.c).
      '-include', 'basicgol_playground.h',
    ];
    const extras = flags.filter((f) => f.startsWith('-W') || f === '-pedantic');

    // O depurador entra só no arquivo do usuário: a biblioteca e o shim
    // compilam exatamente como numa execução normal. `-Wno-uninitialized`
    // cobre as leituras que a própria instrumentação faz (ela mostra o valor
    // de uma variável antes da linha que a preenche).
    const doUsuario = depurar
      ? ['-include', 'basicgol_depurador.h', '-Wno-uninitialized']
      : [];

    if (aoGerarC) {
      const r = await this._executar('clang', [...base, ...extras, '-E', '-o', 'main.i', '-x', 'c', 'main.cc']);
      if (r.codigo === 0) {
        // Os marcadores do clang separam o programa das declarações dos cabeçalhos.
        let noPrograma = false;
        const linhas = new TextDecoder().decode(this._lerArquivo('main.i')).split('\n');
        aoGerarC(linhas.filter((linha) => {
          const marca = /^# \d+ "([^"]+)"/.exec(linha);
          if (!marca) return noPrograma;
          noPrograma = marca[1] === 'main.cc';
          return false;
        }).join('\n').trim());
      }
    }

    const fontes = ['main.cc', 'src/shim.c', ...fontesBiblioteca.map((f) => `src/biblioteca/${f}`)];
    const objetos = [];
    const t0 = performance.now();
    for (const fonte of fontes) {
      // Os objetos ficam no diretório corrente, com nome plano.
      const objeto = `${fonte.split('/').pop().replace(/[^a-zA-Z0-9._-]/g, '_')}.o`;
      const especificas = fonte === 'main.cc' ? doUsuario : [];
      const r = await this._executar('clang',
        [...base, '-emit-obj', ...extras, ...especificas, '-o', objeto, '-x', 'c', fonte]);
      registrar(r.saida.trimEnd());
      if (r.codigo !== 0) {
        registrar(`clang terminou com código ${r.codigo}`);
        return { sucesso: false, registro: registro.join('\n') };
      }
      objetos.push(objeto);
    }
    registrar(`compilação: ${((performance.now() - t0) / 1000).toFixed(2)}s`);

    const t1 = performance.now();
    // A ordem importa: os builtins precisam vir antes de `-lc -lm`.
    const r = await this._executar('lld', [
      '--export-dynamic',
      '-z', 'stack-size=2097152',
      '-L', LIB,
      BUILTINS,
      `${LIB}/crt1.o`,
      ...objetos,
      '-lc', '-lm',
      '-o', 'programa.wasm',
    ]);
    registrar(r.saida.trimEnd());
    if (r.codigo !== 0) {
      registrar(`wasm-ld terminou com código ${r.codigo}`);
      return { sucesso: false, registro: registro.join('\n') };
    }
    registrar(`ligação: ${((performance.now() - t1) / 1000).toFixed(2)}s`);

    const wasm = this._lerArquivo('programa.wasm');
    if (!wasm) {
      registrar('wasm-ld não gerou programa.wasm');
      return { sucesso: false, registro: registro.join('\n') };
    }
    registrar(`programa.wasm: ${(wasm.length / 1024).toFixed(1)} KB (${objetos.length} objetos)`);
    return { sucesso: true, wasm, registro: registro.join('\n') };
  }

  /** Guarda os arquivos de apoio enviados pelo usuário (zip já lido). */
  definirArquivosUsuario(arquivos) {
    this.arquivosUsuario = arquivos || [];
  }
}

/* ------------------------------------------------------------------------- */
/* fonte                                                                      */
/* ------------------------------------------------------------------------- */

/**
 * Prepara o texto que vai para o compilador.
 *
 * A linguagem é C: um programa canônico começa com `#include <basicgol.h>` e
 * tem um bloco `INICIO ... FIM` (o cabeçalho instala as macros). No playground
 * nada disso deveria ser obrigatório para começar a brincar, então:
 *
 *  - o `#include <basicgol.h>` entra sozinho se faltar;
 *  - sem `INICIO` e sem `main`, o corpo é envolvido em `INICIO ... FIM`;
 *  - o resto passa como está: é C de verdade, não uma linguagem inventada.
 */
export function prepararFonte(codigo) {
  const fonte = String(codigo || '');
  const temInclude = /^[ \t]*#[ \t]*include[ \t]*[<"]basicgol\.h[>"]/m.test(fonte);
  const temInicio = /(^|\n)[ \t]*INICIO\b/.test(fonte);
  const temMain = /(^|\n)[ \t]*(int|void)[ \t]+main[ \t]*\(/.test(fonte);

  /** Monta o resultado com o número de linhas acrescentadas no topo. */
  const resultado = (cabecalho, corpo) => {
    const linhas = cabecalho.split('\n').length - 1;
    return { fonte: cabecalho + corpo, deslocamento: linhas };
  };

  if (temMain) {
    return temInclude
      ? { fonte, deslocamento: 0 }
      : resultado('#include <basicgol.h>\n', fonte);
  }

  const corpo = fonte
    .replace(/^[ \t]*#[ \t]*include[ \t]*[<"]basicgol\.h[>"][ \t]*$/m, '')
    .replace(/^[ \t]*#[ \t]*include[ \t]*<stdio\.h>[ \t]*$/m, '');
  if (temInicio) return resultado('#include <basicgol.h>\n#include <stdio.h>\n', corpo);
  return resultado('#include <basicgol.h>\n#include <stdio.h>\nINICIO\n', `${corpo}\nFIM\n`);
}

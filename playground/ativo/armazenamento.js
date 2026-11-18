/*
 * armazenamento.js — sistema de arquivos em memória e o WASI dos programas.
 *
 * O clang e o lld do projeto binji/wasm-clang não falam WASI diretamente: eles
 * delegam as chamadas de caminho/arquivo ao módulo `memfs` (um handler WASI
 * implementado em C), que por sua vez conversa com o host por imports
 * (`copy_in`, `copy_out`, `host_write`, ...). `criarImportsMemfs` implementa o
 * lado do host dessa conversa — é ele que dá ao navegador um "disco" com o
 * programa, o cabeçalho e a biblioteca.
 *
 * `criarWasiPrograma` é outra coisa: o subconjunto de WASI snapshot 0 de que
 * os **programas compilados** precisam (fd_write, fd_read, clock, random). A
 * leitura de stdin pode devolver uma Promise; é isso que permite `LEIA`
 * esperar o usuário digitar sem travar a página (JSPI).
 */

export const BS = { SUCESSO: 0, BADF: 8, NOENT: 44, INVAL: 28, NOSYS: 52 };

const codificador = new TextEncoder();
const decodificador = new TextDecoder();

/** Divide o fluxo de bytes em pedaços UTF-8 válidos (sem cortar acentos). */
export class DecodificadorSeguro {
  constructor() {
    this.parcial = new Uint8Array(0);
    this.dec = new TextDecoder('utf-8');
  }

  /** Recebe bytes e devolve o texto já decodificado (pode ser ''). */
  empurrar(bytes) {
    let dados;
    if (this.parcial.length) {
      dados = new Uint8Array(this.parcial.length + bytes.length);
      dados.set(this.parcial, 0);
      dados.set(bytes, this.parcial.length);
    } else {
      dados = bytes;
    }
    // Mantém no máximo 3 bytes de uma sequência incompleta para a próxima vez.
    let corte = dados.length;
    const limite = Math.max(0, dados.length - 3);
    for (let i = dados.length - 1; i >= limite; i -= 1) {
      const b = dados[i];
      if ((b & 0x80) === 0) {
        corte = i + 1;
        break;
      }
      if ((b & 0xc0) === 0xc0) {
        const precisos = (b & 0xe0) === 0xc0 ? 1 : (b & 0xf0) === 0xe0 ? 2 : 3;
        corte = i + 1 + precisos > dados.length ? i : dados.length;
        break;
      }
    }
    if (corte === 0) corte = dados.length; // não trava em byte inválido
    this.parcial = dados.slice(corte);
    return this.dec.decode(dados.subarray(0, corte), { stream: true });
  }
}

/** Um "disco" simples o bastante para guardar arquivos (usado nos testes). */
export class MemFS {
  constructor() {
    this.raiz = new Map();
  }

  _normalizar(caminho) {
    return String(caminho).split('/').filter((p) => p && p !== '.');
  }

  _no(caminho, criarPasta = false) {
    let atual = this.raiz;
    const partes = this._normalizar(caminho);
    for (let i = 0; i < partes.length; i += 1) {
      const p = partes[i];
      let proximo = atual.get(p);
      if (!proximo) {
        if (!criarPasta) return null;
        proximo = new Map();
        atual.set(p, proximo);
      }
      if (i < partes.length - 1 && !(proximo instanceof Map)) {
        if (!criarPasta) return null;
        proximo = new Map();
        atual.set(p, proximo);
      }
      atual = proximo;
    }
    return atual;
  }

  definirArquivo(caminho, dados) {
    const partes = this._normalizar(caminho);
    const nome = partes.pop();
    const pasta = this._no(`/${partes.join('/')}`, true);
    const bytes = typeof dados === 'string' ? codificador.encode(dados) : dados;
    pasta.set(nome, new Uint8Array(bytes));
  }

  criarPasta(caminho) {
    this._no(caminho, true);
  }

  lerArquivo(caminho) {
    const no = this._no(caminho);
    return no instanceof Uint8Array ? no : null;
  }

  existe(caminho) {
    return this._no(caminho) !== null;
  }

  ehPasta(caminho) {
    return this._no(caminho) instanceof Map;
  }

  remover(caminho) {
    const partes = this._normalizar(caminho);
    const nome = partes.pop();
    const pasta = this._no(`/${partes.join('/')}`);
    if (!(pasta instanceof Map)) return BS.NOENT;
    return pasta.delete(nome) ? BS.SUCESSO : BS.NOENT;
  }

  listar(caminho) {
    const no = this._no(caminho);
    return no instanceof Map ? [...no.keys()] : [];
  }
}

/**
 * Cria o conjunto de imports que o módulo `memfs` espera do host.
 * `obterMemoria()` devolve a memória do clang/lld (o outro lado da conversa).
 */
export function criarImportsMemfs({ obterMemoria, obterMemfs, aoEscrever, aoLogar }) {
  const dv = () => new DataView(obterMemoria().buffer);
  const u8 = (o, n) => new Uint8Array(obterMemoria().buffer, o, n);
  return {
    abort: () => { throw new Error('o disco do compilador abortou'); },
    host_write: (fd, iovs, quantidade, escritoEm) => {
      let total = 0;
      let saida = '';
      let p = iovs;
      const v = dv();
      for (let i = 0; i < quantidade; i += 1) {
        const buf = v.getUint32(p, true); p += 4;
        const tam = v.getUint32(p, true); p += 4;
        saida += decodificador.decode(u8(buf, tam));
        total += tam;
      }
      v.setUint32(escritoEm, total, true);
      if (aoEscrever) aoEscrever(fd, saida);
      return BS.SUCESSO;
    },
    host_read: (fd, iovs, quantidade, lidoEm) => {
      // O compilador e o linker nunca leem stdin.
      dv().setUint32(lidoEm, 0, true);
      return BS.SUCESSO;
    },
    memfs_log: (buf, tam) => {
      // `memfs_log` pode ser chamado antes de o memfs ter memória: daí o guard.
      if (aoLogar && obterMemoria()) aoLogar(decodificador.decode(u8(buf, tam)));
    },
    copy_out: (destino, origem, tam) => {
      u8(destino, tam).set(new Uint8Array(obterMemfs().memoria.buffer, origem, tam));
    },
    copy_in: (destino, origem, tam) => {
      new Uint8Array(obterMemfs().memoria.buffer, destino, tam).set(u8(origem, tam));
    },
  };
}

/**
 * WASI snapshot 0 para um **programa compilado**.
 *
 * `io.ler(bytes)` pode devolver uma Promise: é assim que o `LEIA` espera a
 * digitação no terminal. Quando não há entrada disponível, fd_read devolve 0
 * bytes — o que é EOF para a libc — então o programa termina em vez de travar.
 */
export function criarWasiPrograma({ obterMemoria, io, argv = ['programa'], ambiente = {}, aoSair }) {
  const dv = () => new DataView(obterMemoria().buffer);
  const u8 = (o, n) => new Uint8Array(obterMemoria().buffer, o, n);
  const sair = (codigo) => {
    const e = new Error(`processo terminou com código ${codigo}`);
    e.codigoSaida = codigo;
    if (aoSair) aoSair(codigo);
    throw e;
  };
  const copiarStrings = (lista) => {
    const codificados = lista.map((s) => codificador.encode(s));
    return {
      quantidade: lista.length,
      bytes: codificados.reduce((n, b) => n + b.length + 1, 0),
      escrever(v, argvPtr, bufPtr) {
        let p = bufPtr;
        codificados.forEach((b, i) => {
          v.setUint32(argvPtr + i * 4, p, true);
          u8(p, b.length).set(b);
          u8(p + b.length, 1)[0] = 0;
          p += b.length + 1;
        });
      },
    };
  };
  const entradasAmbiente = Object.entries(ambiente).map(([k, v]) => `${k}=${v}`);

  return {
    proc_exit: sair,
    args_sizes_get: (quantidadePtr, bytesPtr) => {
      const s = copiarStrings(argv);
      dv().setUint32(quantidadePtr, s.quantidade, true);
      dv().setUint32(bytesPtr, s.bytes, true);
      return 0;
    },
    args_get: (argvPtr, bufPtr) => {
      copiarStrings(argv).escrever(dv(), argvPtr, bufPtr);
      return 0;
    },
    environ_sizes_get: (quantidadePtr, bytesPtr) => {
      const s = copiarStrings(entradasAmbiente);
      dv().setUint32(quantidadePtr, s.quantidade, true);
      dv().setUint32(bytesPtr, s.bytes, true);
      return 0;
    },
    environ_get: (envPtr, bufPtr) => {
      copiarStrings(entradasAmbiente).escrever(dv(), envPtr, bufPtr);
      return 0;
    },
    clock_time_get: (id, _precisao, saidaPtr) => {
      const ns = id === 0
        ? BigInt(Math.round(performance.now() * 1e6))
        : BigInt(Date.now()) * 1000000n;
      dv().setBigUint64(saidaPtr, ns, true);
      return 0;
    },
    random_get: (ptr, tam) => {
      const destino = u8(ptr, tam);
      if (globalThis.crypto && globalThis.crypto.getRandomValues) {
        globalThis.crypto.getRandomValues(destino);
      } else {
        for (let i = 0; i < tam; i += 1) destino[i] = (Math.random() * 256) | 0;
      }
      return 0;
    },
    poll_oneoff: () => 0,
    sched_yield: () => 0,
    // Não há disco para o programa: se algum caminho chegar aqui (um
    // `fopen(..., "w")` do usuário, por exemplo), a resposta honesta é "sem
    // sistema de arquivos" em vez de um erro interno.
    path_open: () => BS.NOSYS,
    path_filestat_get: () => BS.NOSYS,
    path_create_directory: () => BS.NOSYS,
    path_remove_directory: () => BS.NOSYS,
    path_unlink_file: () => BS.NOSYS,
    path_rename: () => BS.NOSYS,
    path_symlink: () => BS.NOSYS,
    path_readlink: () => BS.INVAL,
    fd_fdstat_get: (fd, ptr) => {
      if (fd > 2) return BS.BADF;
      // tipo 2 = character device; sem direitos de seek (senão o stdio acha
      // que pode rebobinar stdin e tenta ler tudo de novo).
      const v = dv();
      v.setUint8(ptr, 2);
      v.setUint16(ptr + 2, 0, true);
      v.setBigUint64(ptr + 8, 0n, true);
      v.setBigUint64(ptr + 16, 0xffffffffn, true);
      v.setBigUint64(ptr + 24, 0n, true);
      return 0;
    },
    fd_fdstat_set_flags: () => 0,
    fd_filestat_get: (fd, ptr) => {
      if (fd > 2) return BS.BADF;
      const v = dv();
      v.setBigUint64(ptr, 0n, true);
      v.setBigUint64(ptr + 8, 0n, true);
      v.setBigUint64(ptr + 16, 0n, true);
      v.setUint8(ptr + 24, 2);
      v.setBigUint64(ptr + 32, 0n, true);
      v.setBigUint64(ptr + 40, 0n, true);
      v.setBigUint64(ptr + 48, 0n, true);
      return 0;
    },
    fd_write: (fd, iovs, quantidade, escritoEm) => {
      if (fd > 2) return BS.BADF;
      const v = dv();
      let total = 0;
      for (let i = 0; i < quantidade; i += 1) {
        const buf = v.getUint32(iovs + i * 8, true);
        const tam = v.getUint32(iovs + i * 8 + 4, true);
        io.escrever(u8(buf, tam).slice(), fd === 2 ? 'erro' : 'saida');
        total += tam;
      }
      v.setUint32(escritoEm, total, true);
      return 0;
    },
    fd_read: (fd, iovs, quantidade, lidoEm) => {
      if (fd > 2) return BS.BADF;
      if (fd !== 0) { dv().setUint32(lidoEm, 0, true); return 0; }
      const v = dv();
      const pedacos = [];
      let queremos = 0;
      for (let i = 0; i < quantidade; i += 1) {
        const buf = v.getUint32(iovs + i * 8, true);
        const tam = v.getUint32(iovs + i * 8 + 4, true);
        pedacos.push([buf, tam]);
        queremos += tam;
      }
      const entregar = (chunk) => {
        let resto = chunk;
        for (const [buf, tam] of pedacos) {
          if (!resto.length) break;
          const parte = resto.subarray(0, tam);
          u8(buf, parte.length).set(parte);
          resto = resto.subarray(tam);
        }
        v.setUint32(lidoEm, Math.min(chunk.length, queremos), true);
        return 0;
      };
      const resultado = io.ler(queremos);
      if (resultado && typeof resultado.then === 'function') {
        // Espera o terminal: a instância é suspensa e retomada quando o
        // usuário apertar Enter (JSPI — ver app.js).
        return resultado.then(entregar);
      }
      return entregar(resultado || new Uint8Array(0));
    },
    fd_seek: (fd, _desloc, _origem, novoPtr) => {
      if (fd > 2) return BS.BADF;
      dv().setBigUint64(novoPtr, 0n, true);
      return 0;
    },
    fd_close: () => 0,
    fd_prestat_get: () => BS.BADF,
    fd_prestat_dir_name: () => BS.BADF,
    fd_readdir: () => BS.BADF,
    fd_pread: () => BS.BADF,
    _sair: sair,
  };
}

#!/usr/bin/env node
/*
 * verificar.js — compila e executa os exemplos do playground **sem navegador**,
 * usando o mesmo pacote de ferramentas, os mesmos caminhos e os mesmos
 * argumentos que `ativo/ferramentas.js` passa para clang/wasm-ld.
 *
 * É a prova de fumaça da pilha (toolchain -> wasm -> execução) e a rede de
 * segurança: se um exemplo deixar de compilar, este script acusa. Não substitui
 * `verificar-navegador.mjs` (que importa o código real da página); ele existe
 * para apontar a causa quando aquele falha, com o log inteiro do compilador.
 *
 *   node playground/tools/verificar.js                    # compila todos
 *   node playground/tools/verificar.js --executar          # compila e roda
 *   node playground/tools/verificar.js --dourado            # compila e confere
 *                                                           com *.esperado.txt
 *   node playground/tools/verificar.js --exemplo adivinha.c --entrada "50\n"
 *   node playground/tools/verificar.js --debug             # mostra open/stat
 */
'use strict';

const fs = require('fs');
const path = require('path');
const zlib = require('zlib');

const RAIZ = path.resolve(__dirname, '..', '..');
const PACOTE = path.join(RAIZ, 'playground', 'publico', 'toolchain', 'wasi-cc.tar.gz');
const TRABALHO = path.join(RAIZ, 'playground', '.verificacao');

if (!fs.existsSync(PACOTE)) {
  console.error(`falta ${path.relative(RAIZ, PACOTE)}; rode antes:\n`
    + '  node playground/tools/empacotar-toolchain.js');
  process.exit(2);
}

const EXTRAIR = path.join(TRABALHO, 'est');
const DEPURAR = process.argv.includes('--debug');
const argumento = (nome) => {
  const i = process.argv.indexOf(nome);
  return i >= 0 ? process.argv[i + 1] : null;
};

// ------------------------------------------------------------------ utilidades

/** Lê o tar em memória e chama `aoEntrada(tipo, nome, bytes)`. */
function percorrerTar(bytes, aoEntrada) {
  const dec = new TextDecoder();
  const u8 = bytes;
  const texto = (o, n) => dec.decode(u8.subarray(o, o + n)).replace(/\0[\s\S]*$/, '');
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

/** Carrega `ativo/exemplos.js` como CommonJS (é um `export const` simples). */
function lerExemplos() {
  const fonte = fs.readFileSync(path.join(RAIZ, 'playground', 'ativo', 'exemplos.js'), 'utf8')
    .replace(/^export const EXEMPLOS =/m, 'module.exports =')
    .replace(/^export function acharExemplo[\s\S]*$/m, '')
    .replace(/^export default EXEMPLOS;$/m, '');
  const mod = { exports: {} };
  // eslint-disable-next-line no-new-func
  new Function('module', 'exports', fonte)(mod, mod.exports);
  return mod.exports;
}

// ----------------------------------------------------------------------- memfs

/**
 * O "host" do memfs: descompacta o pacote no disco de trabalho, cria o disco do
 * compilador e responde ao protocolo (`copy_in`, `copy_out`, `host_write`).
 */
function criarAmbiente() {
  let memoriaHost = null;
  let memoriaMemfs = null;
  const dec = new TextDecoder();
  const Buffer8 = (mem, o, n) => new Uint8Array(mem.buffer, o, n);

  const imports = {
    abort: () => { throw new Error('o memfs abortou'); },
    host_write: (fd, iovs, n, escrito) => {
      const dv = new DataView(memoriaHost.buffer);
      let total = 0;
      let p = iovs;
      for (let i = 0; i < n; i += 1) {
        const buf = dv.getUint32(p, true); p += 4;
        const tam = dv.getUint32(p, true); p += 4;
        const saida = dec.decode(Buffer8(memoriaHost, buf, tam));
        if (fd !== 2) process.stdout.write(saida);
        else process.stderr.write(`\x1b[31m${saida}\x1b[0m`);
        total += tam;
      }
      dv.setUint32(escrito, total, true);
      return 0;
    },
    host_read: () => 0,
    memfs_log: (buf, len) => {
      if (!DEPURAR || !memoriaMemfs) return;
      process.stderr.write(`\x1b[35mMEMFS ${dec.decode(Buffer8(memoriaMemfs, buf, len))}\x1b[0m\n`);
    },
    copy_out: (destino, origem, tam) => {
      Buffer8(memoriaHost, destino, tam).set(Buffer8(memoriaMemfs, origem, tam));
    },
    copy_in: (destino, origem, tam) => {
      Buffer8(memoriaMemfs, destino, tam).set(Buffer8(memoriaHost, origem, tam));
    },
  };

  const enc = new TextEncoder();
  return {
    async iniciar(modulo) {
      const inst = await WebAssembly.instantiate(modulo, { env: imports });
      memoriaMemfs = inst.exports.memory;
      const M = inst.exports;
      M.init();

      const colocar = (nome) => {
        const bytes = enc.encode(nome);
        Buffer8(memoriaMemfs, M.GetPathBuf(), bytes.length).set(bytes);
        return bytes.length;
      };

      // Não há "existe?" no memfs: `FindNode` **aborta** quando o caminho não
      // existe. O abort é um `unreachable` do wasm, que o host vê como exceção
      // e que deixa a instância intacta — então usamos a trap como resposta.
      // (Testado: 200 consultas a caminhos inexistentes e o disco continua
      // criando e lendo arquivos normalmente.) O outro caminho possível,
      // `path_filestat_get`, não serve daqui: ele lê o caminho da memória do
      // *host*, que só existe durante a execução do clang/lld.
      const existe = (nome) => {
        const n = colocar(nome);
        try {
          return M.FindNode(n) !== 0xffffffff;
        } catch (e) {
          return false;
        }
      };

      return {
        exports: M,
        pasta(nome) {
          const partes = nome.split('/').filter(Boolean);
          for (let i = 1; i <= partes.length; i += 1) {
            const parcial = partes.slice(0, i).join('/');
            if (existe(parcial)) continue;
            colocar(parcial);
            M.AddDirectoryNode(enc.encode(parcial).length);
          }
        },
        arquivo(nome, dados) {
          const bytes = typeof dados === 'string' ? enc.encode(dados) : dados;
          if (DEPURAR) process.stderr.write(`\x1b[32m+ ${nome} (${bytes.length})\x1b[0m\n`);
          const partes = nome.split('/');
          partes.pop();
          if (partes.length) this.pasta(partes.join('/'));
          const n = colocar(nome);
          const inode = M.AddFileNode(n, bytes.length);
          if (inode < 0) throw new Error(`AddFileNode(${nome}) devolveu ${inode}`);
          Buffer8(memoriaMemfs, M.GetFileNodeAddress(inode), bytes.length).set(bytes);
        },
        ler(nome) {
          if (!existe(nome)) return null;
          const n = colocar(nome);
          const inode = M.FindNode(n);
          return Buffer.from(Buffer8(memoriaMemfs,
            M.GetFileNodeAddress(inode), M.GetFileNodeSize(inode)));
        },
        definirHost(mem) { memoriaHost = mem; },
      };
    },
  };
}

/** Roda clang ou wasm-ld (o memfs responde como `wasi_unstable`). */
async function executarFerramenta(modulo, memfs, nome, argumentos) {
  const lista = [nome, ...argumentos];
  const enc = new TextEncoder();
  let memoria = null;
  const dv = () => new DataView(memoria.buffer);
  const u8 = (o, n) => new Uint8Array(memoria.buffer, o, n);
  const tamanho = lista.reduce((n, s) => n + enc.encode(s).length + 1, 0);

  const wasi = {
    proc_exit: (c) => { const e = new Error('exit'); e.codigo = c; throw e; },
    environ_sizes_get: (a, b) => { dv().setUint32(a, 0, true); dv().setUint32(b, 0, true); return 0; },
    environ_get: () => 0,
    args_sizes_get: (a, b) => { dv().setUint32(a, lista.length, true); dv().setUint32(b, tamanho, true); return 0; },
    args_get: (argv, buf) => {
      let p = buf;
      lista.forEach((s, i) => {
        const b = enc.encode(s);
        dv().setUint32(argv + i * 4, p, true);
        u8(p, b.length).set(b);
        u8(p + b.length, 1)[0] = 0;
        p += b.length + 1;
      });
      return 0;
    },
    random_get: (p, n) => {
      const destino = u8(p, n);
      for (let i = 0; i < n; i += 1) destino[i] = (Math.random() * 256) | 0;
      return 0;
    },
    clock_time_get: (_id, _p, ptr) => { dv().setBigUint64(ptr, BigInt(Date.now()) * 1000000n, true); return 0; },
    poll_oneoff: () => 0,
  };

  const exports = memfs.exports;
  const inst = await WebAssembly.instantiate(modulo, {
    wasi_unstable: { ...wasi, ...exports },
    env: {},
  });
  memoria = inst.exports.memory;
  memfs.definirHost(memoria);

  let codigo = 0;
  try {
    inst.exports._start();
  } catch (e) {
    if (e.codigo === undefined) throw e;
    codigo = e.codigo;
  }
  return { codigo };
}

// ------------------------------------------------------------------ execução

/** Roda o programa compilado com o mesmo WASI que a página usa. */
async function rodar(wasm, entrada) {
  const dec = new TextDecoder();
  const bytesEntrada = Buffer.from(entrada, 'utf8');
  let pos = 0;
  let memoria = null;
  let saida = '';
  let erro = '';
  const dv = () => new DataView(memoria.buffer);
  const u8 = (o, n) => new Uint8Array(memoria.buffer, o, n);

  const wasi = {
    proc_exit: (c) => { const e = new Error('exit'); e.codigo = c; throw e; },
    environ_sizes_get: (a, b) => { dv().setUint32(a, 0, true); dv().setUint32(b, 0, true); return 0; },
    environ_get: () => 0,
    args_sizes_get: (a, b) => { dv().setUint32(a, 1, true); dv().setUint32(b, 7, true); return 0; },
    args_get: (argv, buf) => {
      dv().setUint32(argv, buf, true);
      u8(buf, 7).set(Buffer.from('prog\0\0\0'));
      return 0;
    },
    clock_time_get: (_id, _p, ptr) => { dv().setBigUint64(ptr, BigInt(Date.now()) * 1000000n, true); return 0; },
    random_get: (p, n) => { const d = u8(p, n); for (let i = 0; i < n; i += 1) d[i] = (Math.random() * 256) | 0; return 0; },
    poll_oneoff: () => 0,
    sched_yield: () => 0,
    fd_fdstat_get: (fd, ptr) => {
      if (fd > 2) return 8;
      dv().setUint8(ptr, 2);
      dv().setUint16(ptr + 2, 0, true);
      dv().setBigUint64(ptr + 8, 0n, true);
      dv().setBigUint64(ptr + 16, 0xffffffffn, true);
      dv().setBigUint64(ptr + 24, 0n, true);
      return 0;
    },
    fd_fdstat_set_flags: () => 0,
    fd_filestat_get: (fd, ptr) => {
      if (fd > 2) return 8;
      for (let i = 0; i < 56; i += 1) dv().setUint8(ptr + i, 0);
      dv().setUint8(ptr + 24, 2);
      return 0;
    },
    fd_write: (fd, iovs, n, escrito) => {
      let total = 0;
      for (let i = 0; i < n; i += 1) {
        const buf = dv().getUint32(iovs + i * 8, true);
        const tam = dv().getUint32(iovs + i * 8 + 4, true);
        const texto = dec.decode(u8(buf, tam).slice());
        if (fd === 2) erro += texto; else saida += texto;
        total += tam;
      }
      dv().setUint32(escrito, total, true);
      return 0;
    },
    fd_read: (fd, iovs, n, lido) => {
      let total = 0;
      for (let i = 0; i < n && pos < bytesEntrada.length; i += 1) {
        const buf = dv().getUint32(iovs + i * 8, true);
        const tam = dv().getUint32(iovs + i * 8 + 4, true);
        const pedaco = bytesEntrada.subarray(pos, pos + tam);
        pos += pedaco.length;
        u8(buf, pedaco.length).set(pedaco);
        total += pedaco.length;
      }
      dv().setUint32(lido, total, true);
      return 0;
    },
    fd_seek: (fd, _o, _w, out) => { dv().setBigUint64(out, 0n, true); return 0; },
    fd_close: () => 0,
    fd_prestat_get: () => 8,
    fd_prestat_dir_name: () => 8,
    fd_readdir: () => 8,
    fd_pread: () => 8,
    path_open: () => 52,
    path_filestat_get: () => 52,
    path_create_directory: () => 52,
    path_remove_directory: () => 52,
    path_unlink_file: () => 52,
    path_rename: () => 52,
    path_symlink: () => 52,
    path_readlink: () => 28,
  };

  let codigo = 0;
  const inst = await WebAssembly.instantiate(await WebAssembly.compile(wasm), {
    wasi_unstable: wasi,
    wasi_snapshot_preview1: wasi,
  });
  memoria = inst.exports.memory;
  try {
    inst.exports._start();
  } catch (e) {
    if (e.codigo === undefined) throw e;
    codigo = e.codigo;
  }
  return { codigo, saida, erro };
}

// ---------------------------------------------------------------------- main

(async () => {
  // 1. O pacote é um .tar.gz; descompacta uma vez para o diretório de trabalho.
  fs.rmSync(TRABALHO, { recursive: true, force: true });
  fs.mkdirSync(EXTRAIR, { recursive: true });
  const pacote = new Uint8Array(zlib.gunzipSync(fs.readFileSync(PACOTE)));
  const membros = new Map();
  percorrerTar(pacote, (tipo, nome, dados) => {
    if (tipo !== '5') membros.set(nome, dados);
  });
  for (const [nome, dados] of membros) {
    const destino = path.join(EXTRAIR, nome);
    fs.mkdirSync(path.dirname(destino), { recursive: true });
    fs.writeFileSync(destino, dados);
  }

  // 2. Um único memfs, como na página: ele é o disco de todas as compilações.
  const ambiente = criarAmbiente();
  const memfs = await ambiente.iniciar(await WebAssembly.compile(membros.get('memfs')));
  const clang = await WebAssembly.compile(membros.get('clang'));
  const lld = await WebAssembly.compile(membros.get('lld'));

  process.stdout.write('sysroot: ');
  let arquivos = 0;
  percorrerTar(membros.get('sysroot.tar'), (tipo, nome, dados) => {
    if (tipo === '5') memfs.pasta(`usr/${nome}`);
    else { memfs.arquivo(`usr/${nome}`, dados); arquivos += 1; }
  });
  console.log(`${arquivos} arquivos`);

  // 3. Cabeçalhos, shim, biblioteca e o cabeçalho de compatibilidade.
  for (const nome of [...membros.keys()]) {
    if (nome.startsWith('include/') || nome.startsWith('src/') || nome === 'basicgol_playground.h') {
      memfs.arquivo(nome, membros.get(nome));
    }
  }
  const fontesBiblioteca = [...membros.keys()]
    .filter((n) => n.startsWith('src/biblioteca/') && n.endsWith('.c'))
    .map((n) => n.slice('src/biblioteca/'.length))
    .sort();

  // 4. Argumentos — os mesmos de ativo/ferramentas.js.
  const CABECALHOS = ['usr/include/c++/v1', 'usr/include', 'usr/lib/clang/8.0.1/include'];
  const LIB = 'usr/lib/wasm32-wasi';
  const BUILTINS = 'usr/lib/clang/8.0.1/lib/wasi/libclang_rt.builtins-wasm32.a';
  const base = [
    '-cc1', '-emit-obj', '-disable-free', '-isysroot', '',
    ...CABECALHOS.flatMap((d) => ['-internal-isystem', d]),
    '-ferror-limit', '19', '-fmessage-length', '0', '-fno-color-diagnostics',
    '-mthread-model', 'single', '-std=c11', '-O2', '-Iinclude',
    '-include', 'basicgol_playground.h',
  ];

  const exemplos = lerExemplos();
  const filtro = argumento('--exemplo');
  const entrada = (argumento('--entrada') || '').replace(/\\n/g, '\n');
  const lista = Object.values(exemplos).filter((e) => !filtro
    || e.arquivo === filtro || e.arquivo === `${filtro}.c` || e.titulo === filtro);

  if (!lista.length) {
    console.error(`nenhum exemplo corresponde a "${filtro}"`);
    process.exit(2);
  }

  const conferir = process.argv.includes('--dourado');

  let falhas = 0;
  for (const exemplo of lista) {
    process.stdout.write(`— ${exemplo.arquivo} … `);
    memfs.arquivo('main.cc', exemplo.codigo);
    const objetos = [];
    let ok = true;
    for (const fonte of ['main.cc', 'src/shim.c',
      ...fontesBiblioteca.map((f) => `src/biblioteca/${f}`)]) {
      const objeto = `${fonte.replace(/[^a-zA-Z0-9._-]/g, '_')}.o`;
      const r = await executarFerramenta(clang, memfs, 'clang',
        [...base, '-o', objeto, '-x', 'c', fonte]);
      if (r.codigo !== 0) {
        ok = false;
        console.log(`\n   clang ${r.codigo} em ${fonte}`);
        break;
      }
      objetos.push(objeto);
    }
    if (!ok) { console.log('FALHOU (clang)'); falhas += 1; continue; }

    const r = await executarFerramenta(lld, memfs, 'wasm-ld', [
      '--export-dynamic', '-z', 'stack-size=2097152', '-L', LIB,
      BUILTINS, `${LIB}/crt1.o`, ...objetos, '-lc', '-lm', '-o', 'programa.wasm',
    ]);
    if (r.codigo !== 0) { console.log('FALHOU (wasm-ld)'); falhas += 1; continue; }

    const wasm = memfs.ler('programa.wasm');
    console.log(`ok (${(wasm.length / 1024).toFixed(1)} KB)`);

    if (process.argv.includes('--executar')) {
      const usado = entrada || (exemplo.entrada || '');
      const x = await rodar(wasm, usado);
      const linhas = x.saida.trimEnd().split('\n').slice(0, 6).join('\n      ');
      console.log(`      código ${x.codigo}:\n      ${linhas}`);
      if (x.erro.trim()) console.log(`      stderr: ${x.erro.trim()}`);
    }

    if (conferir) {
      // Teste dourado: a saída tem de ser *idêntica* à de docs/exemplos.
      const esperadoArquivo = path.join(RAIZ, 'docs', 'exemplos',
        exemplo.arquivo.replace(/\.c$/, '.esperado.txt'));
      if (!fs.existsSync(esperadoArquivo)) {
        console.log('      (sem .esperado.txt; nada a conferir)');
        continue;
      }
      const x = await rodar(wasm, entrada || (exemplo.entrada || ''));
      const esperado = fs.readFileSync(esperadoArquivo, 'utf8').replace(/\r\n/g, '\n');
      const obtido = x.saida.replace(/\r\n/g, '\n');
      // `ESCREVA_ERRO` escreve em stderr de propósito (ver utilidades.c) e não
      // faz parte da saída dourada: ela é comparada só com stdout.
      if (obtido === esperado && x.codigo === 0) {
        console.log('      igual ao esperado');
        continue;
      }
      falhas += 1;
      console.log(`      DIFERENTE do esperado (código ${x.codigo})`);
      const linhasEsperadas = esperado.split('\n');
      const linhasObtidas = obtido.split('\n');
      const total = Math.max(linhasEsperadas.length, linhasObtidas.length);
      for (let i = 0; i < total; i += 1) {
        if (linhasEsperadas[i] === linhasObtidas[i]) continue;
        console.log(`        linha ${i + 1}:`);
        console.log(`          esperado: ${JSON.stringify(linhasEsperadas[i])}`);
        console.log(`          obtido:   ${JSON.stringify(linhasObtidas[i])}`);
        if (i > 12) { console.log('        (mais diferenças omitidas)'); break; }
      }
      if (x.erro.trim()) console.log(`        stderr: ${x.erro.trim()}`);
    }
  }

  if (conferir) {
    console.log(falhas
      ? `\n${falhas} exemplo(s) fora do esperado.`
      : '\nTodos os exemplos compilaram e batem com docs/exemplos/*.esperado.txt.');
  } else {
    console.log(falhas ? `\n${falhas} exemplo(s) falharam.` : '\nTodos os exemplos compilaram.');
  }
  process.exit(falhas ? 1 : 0);
})().catch((e) => {
  console.error(e);
  process.exit(1);
});

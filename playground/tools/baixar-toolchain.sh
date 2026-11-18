#!/bin/sh
# baixar-toolchain.sh — baixa o clang/lld em WebAssembly (binji/wasm-clang)
# para playground/.toolchain-build/. Depois rode empacotar-toolchain.js.
set -eu
raiz=$(cd "$(dirname "$0")/../.." && pwd)
destino="$raiz/playground/.toolchain-build"
mkdir -p "$destino"
if [ -x "$destino/wasm-clang-master/clang" ] || [ -f "$destino/wasm-clang-master/clang" ]; then
  echo "já existe: $destino/wasm-clang-master"
  exit 0
fi
echo "baixando binji/wasm-clang (18 MB)..."
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT
curl -fsSL -m 600 "https://codeload.github.com/binji/wasm-clang/tar.gz/refs/heads/master" -o "$tmp/wc.tar.gz"
tar xzf "$tmp/wc.tar.gz" -C "$tmp"
mv "$tmp/wasm-clang-master" "$destino/"
echo "pronto: $destino/wasm-clang-master"

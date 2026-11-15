#!/usr/bin/env sh
# Compile la version web en WebAssembly.
# Prérequis : Emscripten (emcc) dans le PATH — https://emscripten.org/docs/getting_started/
# Résultat : web/dist/ (à ouvrir avec un petit serveur local ou à publier sur GitHub Pages).
set -e
cd "$(dirname "$0")"

mkdir -p dist
cp index.html dist/index.html

emcc encodeur_web.cpp \
  -O2 -std=c++17 \
  -lembind \
  -sMODULARIZE=1 \
  -sEXPORT_NAME=creerEncodeur \
  -sENVIRONMENT=web \
  -sSINGLE_FILE=1 \
  -o dist/encodeur.js

echo "Compilation terminée : web/dist/"

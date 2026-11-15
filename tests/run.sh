#!/usr/bin/env sh
# Compile et lance les tests du cœur, puis vérifie que la version console compile.
# Prérequis : un compilateur C++17 (g++ par défaut ; CXX=clang++ sh tests/run.sh pour en changer).
set -e
cd "$(dirname "$0")/.."

CXX="${CXX:-g++}"
FLAGS="-std=c++17 -Wall -Wextra -Wpedantic -Werror -I src"

mkdir -p build
$CXX $FLAGS tests/tests_chiffrement.cpp src/chiffrement.cpp -o build/tests_chiffrement
$CXX $FLAGS console/main.cpp src/chiffrement.cpp -o build/encodeur-decodeur
./build/tests_chiffrement

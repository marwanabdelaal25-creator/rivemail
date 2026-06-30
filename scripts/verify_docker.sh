#!/usr/bin/env bash
set -eu
make test
python3 scripts/audit_duplicate_bodies.py rivemail
rm -rf build/out
mkdir -p build/out
SRC=$(pwd) OUT=$(pwd)/build/out LIB_FUZZING_ENGINE='-fsanitize=fuzzer' bash .clusterfuzzlite/build.sh
for f in build/out/*_fuzzer; do
  name=$(basename "$f")
  corpus="fuzz/corpus/$name"
  if [ -d "$corpus" ]; then
    "$f" -runs=10 "$corpus"/* >/dev/null 2>&1
  fi
done

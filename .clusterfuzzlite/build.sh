#!/usr/bin/env bash
set -eu
: "${SRC:=$(pwd)}"
: "${OUT:=$SRC/build/out}"
: "${LIB_FUZZING_ENGINE:=-fsanitize=fuzzer}"
CC=${CC:-clang}
BASEFLAGS="${CFLAGS:-} -std=c11 -g -O1 -fno-omit-frame-pointer -I$SRC/include -I$SRC/src"
FASTFLAGS="${CFLAGS:-} -std=c11 -g0 -O0 -fno-omit-frame-pointer -I$SRC/include -I$SRC/src"
SANFLAGS="-fsanitize=address,undefined"
OBJDIR="$SRC/build/cflite"
mkdir -p "$OUT" "$OBJDIR"
ENGINE="$LIB_FUZZING_ENGINE"
EXTRA_MAIN=""
if printf '%s' "$LIB_FUZZING_ENGINE" | grep -q 'fuzzer'; then
  PROBE_C="$OBJDIR/fuzzer_probe.c"
  PROBE_BIN="$OBJDIR/fuzzer_probe"
  printf '#include <stdint.h>\n#include <stddef.h>\nint LLVMFuzzerTestOneInput(const uint8_t*d,size_t n){return 0;}\n' > "$PROBE_C"
  if "$CC" $SANFLAGS "$PROBE_C" $LIB_FUZZING_ENGINE -o "$PROBE_BIN" >/dev/null 2>&1 && "$PROBE_BIN" -runs=0 >/dev/null 2>&1; then
    :
  else
    ENGINE=""
    EXTRA_MAIN="$SRC/fuzz/standalone_main.c"
    if command -v cc >/dev/null 2>&1; then
      CC=${CC_FALLBACK:-cc}
    fi
  fi
fi
OBJS=""
for f in "$SRC"/src/*.c; do
  case "$(basename "$f")" in
    cli.c) continue ;;
  esac
  obj="$OBJDIR/$(basename "$f" .c).o"
  case "$(basename "$f")" in
    analysis_mime.c) "$CC" $BASEFLAGS $SANFLAGS -c "$f" -o "$obj" ;;
    analysis_*.c) "$CC" $FASTFLAGS -c "$f" -o "$obj" ;;
    *) "$CC" $BASEFLAGS $SANFLAGS -c "$f" -o "$obj" ;;
  esac
  OBJS="$OBJS $obj"
done
for target in full_fuzzer header_fuzzer render_fuzzer; do
  "$CC" $BASEFLAGS $SANFLAGS "$SRC/fuzz/$target.c" $EXTRA_MAIN $OBJS $ENGINE -o "$OUT/$target"
done

#!/bin/sh

set -e
cd "$(dirname "$0")"

if [ $# -lt 3 ]; then
  echo "Usage: $0 N PUZZLES file.c [file2.c ...]"
  exit 1
fi
n=$1
puzzles=$2
shift 2
mkdir -p out results
name=results/$(date +%Y-%m-%d_%H%M%S)_$(hostname -s)
csv=$name.csv
echo "version,run,puzzle,solutions,seconds" > "$csv"

{
  echo "# $(date '+%d/%m/%Y %H:%M:%S')  $(hostname -s)  $(uname -sm)"
  echo "# $(gcc --version | head -n 1)  -O0  n=$n  puzzles=$puzzles"
  echo

  for src in "$@"; do
    ver=$(basename "$src" .c)
    bin=out/bench_$ver
    gcc -Wall -Wextra -O0 -o "$bin" "$src"
    "$bin" "$puzzles" count > /dev/null # se salta la primera ejecucion

    # Ejecuta los binarios N veces, cada puzle queda como una fila del csv:
    # "02_main,3,SEND+MORE=MONEY,25,0.022355"
    i=1
    while [ "$i" -le "$n" ]; do
      "$bin" "$puzzles" count | awk -v v="$ver" -v r="$i" '
        /=/          { p = $0 }
        / solutions/ { s = $1 }
        / s \(/      { print v "," r "," p "," s "," $1 }' >> "$csv"
      i=$((i + 1))
    done

    # Calcula el promedio de tiempo medido a lo largo de las N ejecuciones
    echo "== $src (n=$n)"
    grep '=' "$puzzles" | while read -r p; do
      avg=$(awk -F , -v v="$ver" -v p="$p" '$1 == v && $3 == p { s = s + $5; k = k + 1 } END { printf "%.2f", s / k * 1000 }' "$csv")
      printf "%10s ms  %s\n" "$avg" "$p"
    done

    total=$(awk -F , -v v="$ver" -v r="$n" '$1 == v { s = s + $5 } END { printf "%.2f", s / r * 1000 }' "$csv")
    printf "%10s ms  TOTAL\n\n" "$total"
  done
} | tee "$name.txt"
echo "saved: $name.txt $csv"

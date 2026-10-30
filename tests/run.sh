#!/bin/bash
set -eu
root=$(cd "$(dirname "$0")/.." && pwd)
build=$(mktemp -d)
trap 'rm -rf "$build"' EXIT
cd "$build"
for name in regression time error; do
    g++ -std=c++11 -pthread -I"$root" "$root/tests/$name.cc" -o "$name"
done
./error
for mode in smoke-sync smoke-async io-sync io-async stop-full; do
    timeout 10 ./regression "$mode"
done
for mode in second minute-same minute-boundary; do
    mkdir "$mode"
    (cd "$mode" && ../time "$mode")
done
python3 - "$root/README.md" <<'PY'
import pathlib
import sys
text = pathlib.Path(sys.argv[1]).read_text(encoding='utf-8-sig')
pathlib.Path('readme.cc').write_text(text.split('```cpp\n', 1)[1].split('```', 1)[0])
PY
g++ -std=c++11 -pthread -I"$root" readme.cc -o readme
./readme > /dev/null
echo 'All regression checks passed'

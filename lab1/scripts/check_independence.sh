#!/usr/bin/env bash
set -euo pipefail
script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
project_dir="$(cd "$script_dir/.." && pwd)"
cd "$project_dir"
if grep -RInE '#include[[:space:]]*[<"]polynomial/' include/tic_tac_toe src/tic_tac_toe; then
    echo "Домен tic_tac_toe зависит от polynomial" >&2
    exit 1
fi
if grep -RInE '#include[[:space:]]*[<"]tic_tac_toe/' include/polynomial src/polynomial; then
    echo "Домен polynomial зависит от tic_tac_toe" >&2
    exit 1
fi
if grep -RInE 'std::(cin|cout|cerr)' include/tic_tac_toe src/tic_tac_toe include/polynomial src/polynomial; then
    echo "В доменном коде используется глобальный консольный поток" >&2
    exit 1
fi
echo "Домены независимы"

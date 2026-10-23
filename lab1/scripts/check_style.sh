#!/usr/bin/env bash
set -euo pipefail
script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
project_dir="$(cd "$script_dir/.." && pwd)"
cd "$project_dir"
source_dirs=(include src cli tests)
if grep -RInE 'TODO|FIXME|XXX|HACK|^[[:space:]]*#[[:space:]]*if[[:space:]]+0|using[[:space:]]+namespace[[:space:]]+std' "${source_dirs[@]}"; then
    echo "Запрещённый маркер или конструкция найдена" >&2
    exit 1
fi
if grep -RInE '\b(temp|tmp|buffer|help|k1|k2|bcv|draft3)\b' include src cli; then
    echo "Найдено запрещённое или недостаточно осмысленное имя" >&2
    exit 1
fi
if find "${source_dirs[@]}" -type f \( -name '*.h' -o -name '*.cpp' \) -print0 |
   xargs -0 awk 'length($0) > 150 { printf "%s:%d: строка длиннее 150 символов\n", FILENAME, FNR; failed=1 } END { exit failed }'; then
    :
else
    exit 1
fi

echo "Проверка стиля завершена успешно"

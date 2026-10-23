#!/usr/bin/env bash
set -euo pipefail
if [[ $# -ne 1 ]]; then
    echo "Использование: $0 <lab1_cli>" >&2
    exit 2
fi
binary="$(realpath "$1")"
script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if [[ -d "$script_dir/../docs/demo" ]]; then
    demo_dir="$script_dir/../docs/demo"
else
    demo_dir="$(dirname "$binary")/demo"
fi
for scenario in S1 S2 S3; do
    input_file="$demo_dir/${scenario}.in"
    expected_file="$demo_dir/${scenario}.expected"
    if [[ ! -f "$input_file" || ! -f "$expected_file" ]]; then
        echo "Не найден сценарий $scenario в $demo_dir" >&2
        exit 1
    fi
    output_file="$(mktemp)"
    trap 'rm -f "$output_file"' EXIT
    timeout 20 "$binary" < "$input_file" > "$output_file"
    while IFS= read -r expected_line || [[ -n "$expected_line" ]]; do
        [[ -z "$expected_line" ]] && continue
        if ! grep -Fq -- "$expected_line" "$output_file"; then
            echo "$scenario: отсутствует ожидаемая строка: $expected_line" >&2
            cat "$output_file" >&2
            exit 1
        fi
    done < "$expected_file"
    rm -f "$output_file"
done
echo "Smoke-тесты S1–S3 завершились успешно"

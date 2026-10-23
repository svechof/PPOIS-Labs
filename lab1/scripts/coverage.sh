#!/usr/bin/env bash
set -euo pipefail
if [[ $# -ne 2 ]]; then
    echo "Использование: $0 <build-dir> <tic_tac_toe|polynomial|project>" >&2
    exit 2
fi
script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
project_dir="$(cd "$script_dir/.." && pwd)"
build_dir="$(cd "$1" && pwd)"
scope="$2"
cd "$project_dir"
mkdir -p coverage
case "$scope" in
    tic_tac_toe)
        filters=(--filter 'src/tic_tac_toe/.*')
        ;;
    polynomial)
        filters=(--filter 'src/polynomial/.*')
        ;;
    project)
        filters=(--filter 'src/.*' --filter 'cli/cli\.cpp')
        ;;
    *)
        echo "Неизвестная область покрытия: $scope" >&2
        exit 2
        ;;
esac
gcovr --root . --object-directory "$build_dir" \
    "${filters[@]}" \
    --exclude 'cli/main\.cpp' --exclude 'tests/.*' --exclude '.deps/.*' \
    --exclude-throw-branches --print-summary \
    --html-details "coverage/${scope}.html" --fail-under-line 90.01

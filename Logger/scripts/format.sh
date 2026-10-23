#!/bin/sh
# Formats every C++ source with clang-format, or with --check only reports files that need it.
# CI runs the check with the clang-format in the Docker dev image; run it the same way to get
# identical output, since different clang-format versions can format the same code differently:
#   docker run --rm -v "$PWD:/workspace" log-hare-tail:dev scripts/format.sh
set -eu
cd "$(dirname "$0")/.."

files=$(find include src tests main.cpp \( -name '*.h' -o -name '*.cxx' -o -name '*.cpp' \) -print)

if [ "${1:-}" = "--check" ]; then
    # shellcheck disable=SC2086 # word splitting is intended: one argument per file
    clang-format --dry-run --Werror $files
    echo "clang-format: all files are formatted"
else
    # shellcheck disable=SC2086
    clang-format -i $files
fi

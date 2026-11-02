#!/bin/sh
# Builds with --coverage, runs the tests, and reports coverage of the library (src/ and include/).
# Fails when line coverage is below MIN_LINE_COVERAGE. Needs gcc and gcovr, so run it in the dev image:
#   docker run --rm -v "$PWD:/workspace" log-hare-tail:dev scripts/coverage.sh
# Reports land in build/<OS>/coverage/report/ (index.html, coverage.txt, coverage.xml)
set -eu
cd "$(dirname "$0")/.."

# Set just under the current coverage, so it catches regressions; raise it as tests are added
MIN_LINE_COVERAGE=${MIN_LINE_COVERAGE:-85}
build_dir=build/$(uname -s)/coverage
report_dir=$build_dir/report

# Counters from an earlier run would be added to this run's numbers
if [ -d "$build_dir" ]; then
    find "$build_dir" -name '*.gcda' -delete
fi
cmake --workflow --preset coverage

mkdir -p "$report_dir"
gcovr --root . --filter 'src/' --filter 'include/' \
    --exclude-throw-branches --exclude-unreachable-branches \
    --txt "$report_dir/coverage.txt" \
    --html-details "$report_dir/index.html" \
    --cobertura "$report_dir/coverage.xml" \
    --print-summary \
    --fail-under-line "$MIN_LINE_COVERAGE" \
    "$build_dir"

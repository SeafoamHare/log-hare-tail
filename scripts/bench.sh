#!/bin/sh
# Builds the benchmark suite (bench preset) and runs it with fixed settings, so results from
# different commits can be compared. Run it in the bench image:
#   docker run --rm -v "$PWD:/workspace" log-hare-tail:bench scripts/bench.sh [out.json] [benchmark flags...]
# The JSON result goes to out.json, relative to the repository root (default build/<OS>/bench/result.json).
# Other flags are passed on, e.g. --benchmark_filter=BM_Disabled. Compare two results with
# the compare.py that comes with Google Benchmark:
#   python3 build/Linux/bench/_deps/googlebenchmark-src/tools/compare.py benchmarks before.json after.json
set -eu
cd "$(dirname "$0")/.."

build_dir=build/$(uname -s)/bench
out=$build_dir/result.json
case "${1:-}" in
    '' | -*) ;;
    *)
        out=$1
        shift
        ;;
esac

cmake --workflow --preset bench
mkdir -p "$(dirname "$out")"

# 20 repetitions for compare.py's U test, which needs at least 9. Random interleaving spreads
# the machine's slow moments over every benchmark instead of letting them hit just one
"$build_dir/bench/logger_bench" \
    --benchmark_repetitions=20 \
    --benchmark_enable_random_interleaving=true \
    --benchmark_display_aggregates_only=true \
    --benchmark_out_format=json \
    --benchmark_out="$out" \
    "$@"
echo "Results written to $out" >&2

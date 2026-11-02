include(FetchContent)
# Always the pinned download, never an installed copy: results are compared across builds, and
# scripts/bench.sh runs tools/compare.py from this source tree
FetchContent_Declare(
    googlebenchmark
    URL https://github.com/google/benchmark/archive/refs/tags/v1.9.5.tar.gz
    URL_HASH SHA256=9631341c82bac4a288bef951f8b26b41f69021794184ece969f8473977eaa340
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_WERROR OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googlebenchmark)

# Calls the logger only through the public LOG_xxx macros, so the same file builds and runs
# both before and after the producer/consumer refactor (docs/performance-refactor-plan.md)
add_executable(logger_bench logger_bench.cpp)
target_link_libraries(logger_bench PRIVATE logger benchmark::benchmark)
set_target_properties(logger_bench PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
logger_target_defaults(logger_bench)

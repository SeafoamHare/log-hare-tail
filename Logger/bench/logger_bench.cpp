#include <unistd.h>

#include <cstddef>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include <benchmark/benchmark.h>
#include <nlohmann/json.hpp>

#include "core/logger.h"

namespace fs = std::filesystem;

namespace
{
// The loop alone. Subtract it from the others, and compare against it to catch a benchmark
// whose body the compiler removed: that one would report the same time as this
void BM_Baseline(benchmark::State &state)
{
    for (auto _ : state)
    {
    }
}
BENCHMARK(BM_Baseline);

// A level below the runtime level ("level": "INFO" in the config), which should return without
// formatting or writing anything. The logger has no level filtering before the refactor, so
// there this pays the full cost of an enabled record
void BM_DisabledDebug(benchmark::State &state)
{
    for (auto _ : state)
    {
        LOG_DEBUG() << "disabled message " << 42 << std::endl;
    }
}
BENCHMARK(BM_DisabledDebug);

// Same, with an argument that allocates. When the arguments after << are not evaluated for a
// filtered record, this costs the same as BM_DisabledDebug
void BM_DisabledDebugExpensiveArgs(benchmark::State &state)
{
    std::size_t i = 0;
    for (auto _ : state)
    {
        LOG_DEBUG() << "disabled message " << std::to_string(i++) << std::endl;
    }
}
BENCHMARK(BM_DisabledDebugExpensiveArgs);

// An enabled record, written to stdout and to the log file. threads:1 is the single threaded
// case. Real time, because CPU time leaves out the time a thread waits for the lock; with
// several threads, items_per_second is the combined throughput of all of them
void BM_EnabledInfo(benchmark::State &state)
{
    std::size_t i = 0;
    for (auto _ : state)
    {
        LOG_INFO() << "enabled message " << i++ << std::endl;
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_EnabledInfo)->Threads(1)->Threads(2)->Threads(4)->Threads(8)->UseRealTime();
} // namespace

int main(int argc, char **argv)
{
    benchmark::Initialize(&argc, argv);
    if (benchmark::ReportUnrecognizedArguments(argc, argv))
    {
        return 1;
    }

    const fs::path log_dir = fs::temp_directory_path() / ("logger_bench_" + std::to_string(getpid()));
    fs::remove_all(log_dir);
    // The logger before the refactor ignores "level"; the refactored one must honor it.
    // max_size caps the disk space a long run takes, and rotation is part of what is measured
    const nlohmann::json config = {{"log_directory", log_dir.string()},
                                   {"filename", "bench.log"},
                                   {"max_size", 64 * 1024 * 1024},
                                   {"max_files", 2},
                                   {"app_name", "LoggerBench"},
                                   {"level", "INFO"}};
    const fs::path config_file = log_dir.string() + ".json";
    std::ofstream(config_file) << config.dump();
    Logger::getInstance().initialize(config_file.string());
    fs::remove(config_file);

    // The logger prints every record to stdout. Discard it at the file descriptor level, so each
    // record still pays for the write() it makes in production, and show the results on stderr.
    // --benchmark_out still writes its file
    if (std::freopen("/dev/null", "w", stdout) == nullptr)
    {
        std::perror("freopen /dev/null");
        return 1;
    }
    benchmark::ConsoleReporter reporter(isatty(STDERR_FILENO) != 0 ? benchmark::ConsoleReporter::OO_Color
                                                                   : benchmark::ConsoleReporter::OO_None);
    reporter.SetOutputStream(&std::cerr);
    reporter.SetErrorStream(&std::cerr);
    benchmark::RunSpecifiedBenchmarks(&reporter);
    benchmark::Shutdown();

    fs::remove_all(log_dir);
    return 0;
}

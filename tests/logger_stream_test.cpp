#include <unistd.h>

#include <filesystem>
#include <fstream>
#include <regex>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "core/logger.h"

namespace fs = std::filesystem;

// Logger is a singleton, so it is initialized once for the whole suite with a
// config that points at a fresh temp directory
class LoggerStreamTest : public ::testing::Test
{
protected:
    static fs::path log_dir_;
    static fs::path log_file_;

    static void SetUpTestSuite()
    {
        log_dir_ = fs::temp_directory_path() / ("logger_stream_test_" + std::to_string(getpid()));
        fs::remove_all(log_dir_);
        log_file_ = log_dir_ / "test.log";

        // max_size is large so rotation never splits the records under test
        nlohmann::json config = {{"log_directory", log_dir_.string()},
                                 {"filename", "test.log"},
                                 {"max_size", 10 * 1024 * 1024},
                                 {"max_files", 2},
                                 {"app_name", "LoggerStreamTest"}};
        fs::path config_file = fs::temp_directory_path() / ("logger_stream_test_" + std::to_string(getpid()) + ".json");
        std::ofstream(config_file) << config.dump();

        Logger::getInstance().initialize(config_file.string());
        fs::remove(config_file);
    }

    static void TearDownTestSuite() { fs::remove_all(log_dir_); }

    // Message text of every plain text record written to the log file so far.
    // A record looks like:
    //   2026-10-22 16:11:14.753 [INFO] [TestBody] A (file: tests/x.cpp, line: 7) (app: ...) (pod.id: ...)
    // so the message is what sits between "[<function>] " and " (file: "
    static std::vector<std::string> readMessages()
    {
        static const std::regex record(R"(^\S+ \S+ \[[A-Z]+\] \[[^\]]*\] (.*) \(file: )");
        std::vector<std::string> messages;
        std::ifstream file(log_file_);
        std::string line;
        while (std::getline(file, line))
        {
            std::smatch match;
            if (std::regex_search(line, match, record))
            {
                messages.push_back(match[1]);
            }
            else
            {
                ADD_FAILURE() << "unexpected log line format: " << line;
            }
        }
        return messages;
    }
};

fs::path LoggerStreamTest::log_dir_;
fs::path LoggerStreamTest::log_file_;

// Purpose:  std::endl resets the thread_local stream after writing a record, so the
//           next message does not carry the previous message's text
// Input:    LOG_INFO() << "A" << std::endl;  then  LOG_WARN() << "B" << std::endl;
// Expected: exactly 2 new records, "A" then "B" (not "A" then "AB")
TEST_F(LoggerStreamTest, ConsecutiveLogsDoNotAccumulate)
{
    size_t before = readMessages().size();

    LOG_INFO() << "A" << std::endl;
    LOG_WARN() << "B" << std::endl;

    std::vector<std::string> messages = readMessages();
    ASSERT_EQ(messages.size(), before + 2);
    EXPECT_EQ(messages[before], "A");
    EXPECT_EQ(messages[before + 1], "B");
}

// Purpose:  values of different types chained with << before std::endl are joined into
//           one record, and only std::endl ends a record
// Input:    LOG_INFO() << "id=" << 42 << " done" << std::endl;  then  LOG_INFO() << "next" << std::endl;
// Expected: exactly 2 new records, "id=42 done" then "next"
TEST_F(LoggerStreamTest, MultipleValuesFormOneMessage)
{
    size_t before = readMessages().size();

    LOG_INFO() << "id=" << 42 << " done" << std::endl;
    LOG_INFO() << "next" << std::endl;

    std::vector<std::string> messages = readMessages();
    ASSERT_EQ(messages.size(), before + 2);
    EXPECT_EQ(messages[before], "id=42 done");
    EXPECT_EQ(messages[before + 1], "next");
}

// Purpose:  threads logging at the same time build messages in their own thread_local
//           stream, so pieces from different threads never mix and no record is lost
// Input:    4 threads, each logging 20 times: LOG_INFO() << "t" << t << "-" << i << std::endl;
// Expected: exactly 80 new records, and each of "t0-0" ... "t3-19" appears exactly once
//           (order is not checked because threads interleave)
TEST_F(LoggerStreamTest, EachThreadKeepsItsOwnMessages)
{
    size_t before = readMessages().size();
    const int num_threads = 4;
    const int logs_per_thread = 20;

    std::vector<std::thread> threads;
    for (int t = 0; t < num_threads; ++t)
    {
        threads.emplace_back(
            [t]()
            {
                for (int i = 0; i < logs_per_thread; ++i)
                {
                    LOG_INFO() << "t" << t << "-" << i << std::endl;
                }
            });
    }
    for (auto &thread : threads)
    {
        thread.join();
    }

    // Threads interleave, so check that every expected message appears exactly once
    std::vector<std::string> messages = readMessages();
    ASSERT_EQ(messages.size(), before + num_threads * logs_per_thread);
    std::multiset<std::string> actual(messages.begin() + before, messages.end());
    for (int t = 0; t < num_threads; ++t)
    {
        for (int i = 0; i < logs_per_thread; ++i)
        {
            std::string expected = "t" + std::to_string(t) + "-" + std::to_string(i);
            EXPECT_EQ(actual.count(expected), 1u) << "missing or duplicated: " << expected;
        }
    }
}

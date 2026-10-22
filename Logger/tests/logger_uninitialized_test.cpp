#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <vector>
#include "core/logger.h"

// This file never calls Logger::initialize(). Logger is a singleton, so this must stay
// in its own test executable (the Makefile builds one per tests/*.cpp)

static size_t countOccurrences(const std::string &text, const std::string &needle)
{
    size_t count = 0;
    for (size_t pos = text.find(needle); pos != std::string::npos; pos = text.find(needle, pos + needle.size()))
    {
        ++count;
    }
    return count;
}

// Purpose:  logging before initialize() must not crash (it used to dereference a null
//           LogStoreController); records still reach the console, and stderr gets a
//           warning only once, like glog's "Logging before InitGoogleLogging()"
// Input:    3 x LOG_INFO() << "before init " << i << std::endl;  without initialize()
// Expected: no crash, 3 JSON records on stdout with the right messages,
//           and the warning appears exactly once on stderr
TEST(LoggerUninitializedTest, LogsToConsoleAndWarnsOnce)
{
    testing::internal::CaptureStdout();
    testing::internal::CaptureStderr();

    for (int i = 0; i < 3; ++i)
    {
        LOG_INFO() << "before init " << i << std::endl;
    }

    std::string out = testing::internal::GetCapturedStdout();
    std::string err = testing::internal::GetCapturedStderr();

    std::vector<std::string> messages;
    std::istringstream lines(out);
    std::string line;
    while (std::getline(lines, line))
    {
        messages.push_back(nlohmann::json::parse(line)["message.context"]);
    }
    ASSERT_EQ(messages.size(), 3u);
    EXPECT_EQ(messages[0], "before init 0");
    EXPECT_EQ(messages[2], "before init 2");

    EXPECT_EQ(countOccurrences(err, "Logging before Logger::initialize()"), 1u) << "stderr was: " << err;
}

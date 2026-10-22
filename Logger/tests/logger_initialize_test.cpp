#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include "core/logger.h"

namespace fs = std::filesystem;

// Logger is a singleton and initialize() can succeed only once per process, so the
// whole sequence is checked in one test instead of relying on test order
class LoggerInitializeTest : public ::testing::Test
{
protected:
    fs::path log_dir_;
    fs::path config_file_;

    void SetUp() override
    {
        std::string tag = "logger_initialize_test_" + std::to_string(getpid());
        log_dir_ = fs::temp_directory_path() / tag;
        fs::remove_all(log_dir_);
        config_file_ = fs::temp_directory_path() / (tag + ".json");

        nlohmann::json config = {
            {"log_directory", log_dir_.string()},
            {"filename", "test.log"},
            {"max_size", 10 * 1024 * 1024},
            {"max_files", 2},
            {"app_name", "LoggerInitializeTest"}};
        std::ofstream(config_file_) << config.dump();
    }

    void TearDown() override
    {
        fs::remove(config_file_);
        fs::remove_all(log_dir_);
    }

    std::string readLogFile() const
    {
        std::ifstream file(log_dir_ / "test.log");
        return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }
};

// Purpose:  a failed initialize() does not count as initialized, the first successful
//           call works, and a second call throws instead of swapping the log file
// Input:    initialize("missing.json"), then initialize(config), then initialize(config) again,
//           logging after each step
// Expected: the missing config throws std::runtime_error; the valid config succeeds;
//           the repeated call throws std::logic_error; logging still goes to the original file
TEST_F(LoggerInitializeTest, SecondCallThrowsAndFailedCallCanBeRetried)
{
    EXPECT_THROW(Logger::getInstance().initialize((log_dir_ / "missing.json").string()), std::runtime_error);

    ASSERT_NO_THROW(Logger::getInstance().initialize(config_file_.string()));
    LOG_INFO() << "after first init" << std::endl;

    EXPECT_THROW(Logger::getInstance().initialize(config_file_.string()), std::logic_error);
    LOG_INFO() << "after second init" << std::endl;

    std::string content = readLogFile();
    EXPECT_NE(content.find("after first init"), std::string::npos);
    EXPECT_NE(content.find("after second init"), std::string::npos);
}

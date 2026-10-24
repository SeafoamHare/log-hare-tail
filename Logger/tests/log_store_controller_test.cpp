#include <unistd.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "core/log_store_controller.h"

namespace fs = std::filesystem;

// LogStoreController is not a singleton, so every test builds its own instance on a
// fresh temp directory
class LogStoreControllerTest : public ::testing::Test
{
protected:
    fs::path log_dir_;
    fs::path config_file_;

    void SetUp() override
    {
        std::string tag = "log_store_controller_test_" + std::to_string(getpid());
        log_dir_ = fs::temp_directory_path() / tag;
        fs::remove_all(log_dir_);
        config_file_ = fs::temp_directory_path() / (tag + ".json");
    }

    void TearDown() override
    {
        fs::remove(config_file_);
        fs::remove_all(log_dir_);
    }

    Config_File makeConfig(std::uint64_t max_size, int max_files)
    {
        nlohmann::json config = {{"log_directory", log_dir_.string()},
                                 {"filename", "test.log"},
                                 {"max_size", max_size},
                                 {"max_files", max_files},
                                 {"app_name", "LogStoreControllerTest"}};
        std::ofstream(config_file_) << config.dump();
        return Config_File(config_file_.string());
    }

    fs::path logPath(const std::string &name) const { return log_dir_ / name; }

    std::string readFile(const std::string &name) const
    {
        std::ifstream file(logPath(name));
        return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }
};

// Purpose:  max_size values that do not fit in an int must not be truncated, otherwise
//           4 GiB becomes 0 and 4 GiB + 64 becomes 64, and the file rotates almost
//           on every write
// Input:    max_size = 4 GiB and 4 GiB + 64, then 3 short writes
// Expected: no rotation, all 3 lines stay in test.log and 0.test.log is never created
TEST_F(LogStoreControllerTest, MaxSizeAboveIntRangeIsNotTruncated)
{
    const std::uint64_t four_gib = 4ULL * 1024 * 1024 * 1024;
    for (std::uint64_t max_size : {four_gib, four_gib + 64})
    {
        SCOPED_TRACE("max_size = " + std::to_string(max_size));
        fs::remove_all(log_dir_);
        {
            Config_File config = makeConfig(max_size, 3);
            LogStoreController store(config);
            store.Write("line 1 of a message that is longer than sixty-four bytes in total");
            store.Write("line 2");
            store.Write("line 3");
        }

        EXPECT_FALSE(fs::exists(logPath("0.test.log")));
        EXPECT_EQ(readFile("test.log"),
                  "line 1 of a message that is longer than sixty-four bytes in total\nline 2\nline 3\n");
    }
}

// Purpose:  once the current file reaches max_size it is moved to 0.test.log and a new
//           empty test.log is started
// Input:    max_size = 20, then "0123456789" (11 bytes with newline), "abcdefghij" (11 bytes),
//           "next"
// Expected: the second write reaches 22 >= 20 bytes and rotates, so 0.test.log holds the
//           first two lines and test.log holds only "next"
TEST_F(LogStoreControllerTest, RotatesWhenMaxSizeIsReached)
{
    {
        Config_File config = makeConfig(20, 3);
        LogStoreController store(config);
        store.Write("0123456789");
        EXPECT_FALSE(fs::exists(logPath("0.test.log")));
        store.Write("abcdefghij");
        store.Write("next");
    }

    EXPECT_EQ(readFile("0.test.log"), "0123456789\nabcdefghij\n");
    EXPECT_EQ(readFile("test.log"), "next\n");
}

// Purpose:  max_files counts the current file too, older backups shift up by one on each
//           rotation, and the oldest one is dropped
// Input:    max_files = 3, max_size = 1 so every write rotates, then writes "a" ... "e"
// Expected: exactly test.log (empty), 0.test.log = "e" (newest) and 1.test.log = "d";
//           2.test.log is never created
TEST_F(LogStoreControllerTest, KeepsAtMostMaxFiles)
{
    {
        Config_File config = makeConfig(1, 3);
        LogStoreController store(config);
        for (const char *msg : {"a", "b", "c", "d", "e"})
        {
            store.Write(msg);
        }
    }

    EXPECT_EQ(readFile("test.log"), "");
    EXPECT_EQ(readFile("0.test.log"), "e\n");
    EXPECT_EQ(readFile("1.test.log"), "d\n");
    EXPECT_FALSE(fs::exists(logPath("2.test.log")));
}

// Purpose:  max_files = 1 means only the current file is kept, so rotation drops it
//           instead of moving it to 0.test.log
// Input:    max_files = 1, max_size = 1 so every write rotates, then writes "a" and "b"
// Expected: test.log is empty after each rotation and 0.test.log is never created
TEST_F(LogStoreControllerTest, MaxFilesOneKeepsNoBackup)
{
    {
        Config_File config = makeConfig(1, 1);
        LogStoreController store(config);
        store.Write("a");
        store.Write("b");
    }

    EXPECT_EQ(readFile("test.log"), "");
    EXPECT_FALSE(fs::exists(logPath("0.test.log")));
}

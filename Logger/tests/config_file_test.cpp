#include <unistd.h>

#include <filesystem>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

#include "core/config_file.h"

namespace fs = std::filesystem;

class ConfigFileTestBase
{
protected:
    fs::path log_dir_;
    fs::path config_file_;

    void createPaths()
    {
        std::string tag = "config_file_test_" + std::to_string(getpid());
        log_dir_ = fs::temp_directory_path() / tag;
        fs::remove_all(log_dir_);
        config_file_ = fs::temp_directory_path() / (tag + ".json");
    }

    void removePaths()
    {
        fs::remove(config_file_);
        fs::remove_all(log_dir_);
    }

    nlohmann::json validConfig() const
    {
        return {{"log_directory", log_dir_.string()},
                {"filename", "test.log"},
                {"max_size", 1024},
                {"max_files", 3},
                {"app_name", "ConfigFileTest"}};
    }

    void writeConfig(const nlohmann::json &config) const { std::ofstream(config_file_) << config.dump(); }
};

class ConfigFileTest : public ::testing::Test, protected ConfigFileTestBase
{
protected:
    void SetUp() override { createPaths(); }
    void TearDown() override { removePaths(); }
};

// Purpose:  1 is the smallest allowed value for both max_size and max_files
// Input:    max_size = 1, max_files = 1
// Expected: the config loads and keeps both values
TEST_F(ConfigFileTest, AcceptsMinimumValues)
{
    nlohmann::json config = validConfig();
    config["max_size"] = 1;
    config["max_files"] = 1;
    writeConfig(config);

    Config_File loaded(config_file_.string());
    EXPECT_EQ(loaded.max_size, 1u);
    EXPECT_EQ(loaded.max_files, 1);
}

// Purpose:  the largest value of each field's type is still accepted, so the upper bound
//           follows the type and is not off by one
// Input:    max_size = numeric_limits<decltype(max_size)>::max() (SIZE_MAX),
//           max_files = numeric_limits<decltype(max_files)>::max() (INT_MAX)
// Expected: the config loads and keeps both values
TEST_F(ConfigFileTest, AcceptsMaximumValuesOfFieldTypes)
{
    const auto max_size = std::numeric_limits<decltype(Config_File::max_size)>::max();
    const auto max_files = std::numeric_limits<decltype(Config_File::max_files)>::max();
    nlohmann::json config = validConfig();
    config["max_size"] = max_size;
    config["max_files"] = max_files;
    writeConfig(config);

    Config_File loaded(config_file_.string());
    EXPECT_EQ(loaded.max_size, max_size);
    EXPECT_EQ(loaded.max_files, max_files);
}

// Purpose:  an integer too large even for uint64_t is rejected. nlohmann::json stores it as
//           a double (1.8446744073709552e+19), and converting an out-of-range double to an
//           integer is undefined behavior, so the check must reject it before converting
// Input:    the raw JSON text "max_size": 18446744073709551616 (UINT64_MAX + 1), written as
//           text because no C++ integer type can hold it
// Expected: std::runtime_error whose message names "max_size"
TEST_F(ConfigFileTest, RejectsMaxSizeAboveUint64Max)
{
    std::string text = validConfig().dump();
    const std::string original = "\"max_size\":1024";
    ASSERT_NE(text.find(original), std::string::npos) << text;
    text.replace(text.find(original), original.size(), "\"max_size\":18446744073709551616");
    std::ofstream(config_file_) << text;

    try
    {
        Config_File loaded(config_file_.string());
        FAIL() << "expected std::runtime_error, but max_size loaded as " << loaded.max_size;
    }
    catch (const std::runtime_error &e)
    {
        EXPECT_NE(std::string(e.what()).find("\"max_size\""), std::string::npos) << e.what();
    }
}

struct InvalidField
{
    std::string key;
    nlohmann::json value; // null means the key is removed from the config
    std::string label;
};

// Shows the parameter as key=value in failure messages instead of raw bytes
void PrintTo(const InvalidField &field, std::ostream *os)
{
    *os << field.key << "=" << (field.value.is_null() ? "<missing>" : field.value.dump());
}

class ConfigFileInvalidFieldTest : public ::testing::TestWithParam<InvalidField>, protected ConfigFileTestBase
{
protected:
    void SetUp() override { createPaths(); }
    void TearDown() override { removePaths(); }
};

// Purpose:  max_size and max_files outside 1..max are rejected when the config is loaded,
//           instead of wrapping around (for example -1 becoming SIZE_MAX, so the file
//           never rotates) or failing later with an unclear nlohmann::json error
// Input:    a valid config with one field missing, 0, negative, fractional, a string,
//           or above the int range for max_files
// Expected: std::runtime_error whose message names the field
TEST_P(ConfigFileInvalidFieldTest, ThrowsWithFieldName)
{
    const InvalidField &field = GetParam();
    nlohmann::json config = validConfig();
    if (field.value.is_null())
    {
        config.erase(field.key);
    }
    else
    {
        config[field.key] = field.value;
    }
    writeConfig(config);

    try
    {
        Config_File loaded(config_file_.string());
        FAIL() << "expected std::runtime_error";
    }
    catch (const std::runtime_error &e)
    {
        EXPECT_NE(std::string(e.what()).find("\"" + field.key + "\""), std::string::npos) << e.what();
    }
}

INSTANTIATE_TEST_SUITE_P(OutOfRange, ConfigFileInvalidFieldTest,
                         ::testing::Values(InvalidField{"max_size", nullptr, "max_size_missing"},
                                           InvalidField{"max_size", 0, "max_size_zero"},
                                           InvalidField{"max_size", -1, "max_size_negative"},
                                           InvalidField{"max_size", 1.5, "max_size_fraction"},
                                           InvalidField{"max_size", "1024", "max_size_string"},
                                           InvalidField{"max_files", nullptr, "max_files_missing"},
                                           InvalidField{"max_files", 0, "max_files_zero"},
                                           InvalidField{"max_files", -1, "max_files_negative"},
                                           InvalidField{"max_files", 2147483648LL, "max_files_above_int_max"}),
                         [](const ::testing::TestParamInfo<InvalidField> &info) { return info.param.label; });

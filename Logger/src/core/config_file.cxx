#include "core/config_file.h"

#include <cstdint>
#include <limits>

// Reads an integer field that must be between 1 and max. A missing field, 0, a negative
// number, a fraction or a string is rejected with a message naming the field, instead of
// wrapping around when converted to an unsigned type
static std::uint64_t readPositiveInteger(const json &j, const std::string &key, std::uint64_t max)
{
    if (!j.contains(key) || !j.at(key).is_number_unsigned() || j.at(key).get<std::uint64_t>() == 0 ||
        j.at(key).get<std::uint64_t>() > max)
    {
        throw std::runtime_error("Invalid config: \"" + key + "\" must be an integer between 1 and " +
                                 std::to_string(max));
    }
    return j.at(key).get<std::uint64_t>();
}

Config_File::Config_File(const std::string &config_file)
{
    std::ifstream file(config_file);
    if (!file.is_open())
    {
        throw std::runtime_error("Unable to open config file: " + config_file);
    }

    json j;
    file >> j;
    log_directory = j["log_directory"];
    filename = j["filename"];
    // The upper bound comes from each field's type, so changing the type in config_file.h
    // cannot leave a bound here that lets larger values through and truncates them
    max_size = static_cast<decltype(max_size)>(
        readPositiveInteger(j, "max_size", std::numeric_limits<decltype(max_size)>::max()));
    max_files = static_cast<decltype(max_files)>(
        readPositiveInteger(j, "max_files", std::numeric_limits<decltype(max_files)>::max()));
    app_name = j["app_name"];

    if (!createDirectory(log_directory))
    {
        throw std::runtime_error("Unable to create log directory: " + log_directory +
                                 ", error: " + std::strerror(errno));
    }
};

bool Config_File::createDirectory(const std::string &path)
{
    size_t pos = 0;
    std::string dir;
    if (path[0] == '/')
    {
        pos = 1;
        dir = "/";
    }

    while ((pos = path.find('/', pos)) != std::string::npos)
    {
        dir = path.substr(0, pos);
        if (!dir.empty())
        {
            int status = mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            if (status != 0 && errno != EEXIST)
            {
                return false;
            }
        }
        pos++;
    }

    int status = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    return (status == 0 || errno == EEXIST);
};

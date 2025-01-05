#include "core/config_file.h"

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
    max_size = j["max_size"];
    max_files = j["max_files"];
    app_name = j["app_name"];

    // Ensure the log directory exists
    if (!createDirectory(log_directory))
    {
        throw std::runtime_error("Unable to create log directory: " + log_directory + ", error: " + std::strerror(errno));
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

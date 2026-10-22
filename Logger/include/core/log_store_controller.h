#ifndef LOG_STORE_CONTROLLER_H
#define LOG_STORE_CONTROLLER_H

#include <fstream>
#include <memory>
#include "config_file.h"
class LogStoreController
{
private:
    std::ofstream file_;
    std::string file_directory_;
    std::string file_filename_;
    std::string file_full_path_;
    // Same types as in Config_File, so every value the config accepts fits here
    decltype(Config_File::max_size) file_max_size_;
    decltype(Config_File::max_files) file_max_create_count_;
    void Rotate();

public:
    LogStoreController(Config_File &config);
    ~LogStoreController();
    bool IsOpen();
    void Write(const std::string &logmsg);
};

#endif // LOG_STORE_CONTROLLER_H
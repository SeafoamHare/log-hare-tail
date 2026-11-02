#ifndef LOG_HARE_TAIL_LOG_STORE_CONTROLLER_H
#define LOG_HARE_TAIL_LOG_STORE_CONTROLLER_H

#include <fstream>
#include <memory>

#include "log_hare_tail/config_file.h"
class LogStoreController
{
private:
    std::ofstream file_;
    std::string file_directory_;
    std::string file_filename_;
    std::string file_full_path_;
    decltype(Config_File::max_size) file_max_size_;
    decltype(Config_File::max_files) file_max_create_count_;
    void Rotate();

public:
    LogStoreController(Config_File &config);
    ~LogStoreController();
    // Owns an open file, so copying would leave two objects writing and rotating the same file
    LogStoreController(const LogStoreController &) = delete;
    LogStoreController &operator=(const LogStoreController &) = delete;
    bool IsOpen();
    void Write(const std::string &logmsg);
};

#endif // LOG_HARE_TAIL_LOG_STORE_CONTROLLER_H

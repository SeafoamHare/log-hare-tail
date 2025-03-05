#include "core/log_store_controller.h"
#include <filesystem>

LogStoreController::LogStoreController(Config_File &config)
{
    file_directory_ = config.log_directory;
    file_filename_ = config.filename;
    file_full_path_ = config.log_directory + "/" + config.filename;
    file_max_size_ = config.max_size;
    file_max_create_count_ = config.max_files;
    file_.open(file_full_path_, std::ios::app);
    if (!file_.is_open())
    {
        throw std::runtime_error("Unable to open log file: " + file_full_path_);
    }
}

LogStoreController::~LogStoreController()
{
    if (file_.is_open())
    {
        file_.close();
    }
}

void LogStoreController::Write(const std::string &logmsg)
{
    if (file_.is_open())
    {
        file_ << logmsg << std::endl;
        file_.flush();

        file_.seekp(0, std::ios::end);
        if (static_cast<size_t>(file_.tellp()) >= file_max_size_)
        {
            Rotate();
        }
    }
}

void LogStoreController::Rotate()
{
    file_.close();

    // Rename existing log files
    for (int i = file_max_create_count_ - 2; i > 0; --i)
    {
        std::string old_name = file_directory_ + "/" + std::to_string(i - 1) + "." + file_filename_;
        std::string new_name = file_directory_ + "/" + std::to_string(i) + "." + file_filename_;
        rename(old_name.c_str(), new_name.c_str());
    }

    // Rename the current log file
    // std::string backup_name = "0." + m_full_path;
    std::string backup_name = file_directory_ + "/0." + file_filename_;
    rename(file_full_path_.c_str(), backup_name.c_str());

    // Open a new log file
    file_.open(file_full_path_, std::ios::app);
    if (!file_.is_open())
    {
        throw std::runtime_error("Unable to open log file: " + file_full_path_);
    }
}

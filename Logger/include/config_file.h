#ifndef CONFIG_FILE_H
#define CONFIG_FILE_H

#include <string>
#include <stdexcept>
#include <fstream>
#include <sys/stat.h>
#include <cstring>
#include <cerrno>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Config_File
{
public:
    // 構造函數，從配置檔案讀取配置
    explicit Config_File(const std::string &config_file);

    // 配置文件相關屬性
    std::string filename;        // 日誌文件名
    size_t max_size;             // 日誌文件的最大大小
    int max_files;               // 最多保留的日誌文件數量
    std::string app_name;        // 應用程序名稱
    std::string log_directory;   // 日誌存放目錄

private:
    // 創建目錄，並確保父目錄也存在
    bool createDirectory(const std::string &path);
};

#endif // CONFIG_FILE_H

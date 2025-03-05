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
  explicit Config_File(const std::string &config_file);

  // Configuration file related properties
  std::string filename;      // Name of the log file
  size_t max_size;           // Maximum size of each log file in bytes
  int max_files;             // Maximum number of backup files to keep
  std::string app_name;      // Name of the application
  std::string log_directory; // Directory where log files will be stored

private:
  bool createDirectory(const std::string &path);
};

#endif // CONFIG_FILE_H

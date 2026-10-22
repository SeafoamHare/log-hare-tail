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
  // The types of max_size and max_files set the largest value the config accepts.
  // LogStoreController and the config check use decltype to follow them, so change
  // the type only here.
  size_t max_size;           // Size in bytes at which the current log file is rotated, at least 1
  int max_files;             // Total number of log files including the current one, at least 1
  std::string app_name;      // Name of the application
  std::string log_directory; // Directory where log files will be stored

private:
  bool createDirectory(const std::string &path);
};

#endif // CONFIG_FILE_H

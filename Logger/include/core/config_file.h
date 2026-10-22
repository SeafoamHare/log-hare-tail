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
  std::string filename;
  size_t max_size;
  int max_files;
  std::string app_name;      
  std::string log_directory; 

private:
  bool createDirectory(const std::string &path);
};

#endif // CONFIG_FILE_H

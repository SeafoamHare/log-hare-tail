#ifndef LOG_HARE_TAIL_MODEL_LOG_INFO_MODEL_H
#define LOG_HARE_TAIL_MODEL_LOG_INFO_MODEL_H

#include <string>

#include "log_hare_tail/log_level.h"
struct logInfoModel
{
public:
    std::string message;
    std::string device_id;
    LogLevel level = LogLevel::INFO;
    std::string file;
    int line = 0;
    std::string function;
};

#endif // LOG_HARE_TAIL_MODEL_LOG_INFO_MODEL_H

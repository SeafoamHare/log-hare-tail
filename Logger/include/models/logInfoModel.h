#ifndef LOGINFOMODEL_H
#define LOGINFOMODEL_H

#include <string>

#include "../core/LogLevel.h"
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

#endif // LOGINFOMODEL_H
#ifndef LOGINFOMODEL_H
#define LOGINFOMODEL_H

#include <string>
#include <chrono>
#include <ctime>
#include <sstream>
#include "../core/LogLevel.h"
#include <iomanip>
struct logInfoModel
{
public:
    std::string message;
    std::string device_id;
    LogLevel level;
    std::string file;
    int line;
    std::string function;
    std::string timestamp;

    // logInfoModel(const std::string &msg, const std::string &dev_id, LogLevel lvl,
    //              const std::string &fname, int ln)
    //     : message(msg), device_id(dev_id), level(lvl), file(fname), line(ln), timestamp(getCurrentTimestamp()) {}

private:
    std::string getCurrentTimestamp()
    {
        auto now = std::chrono::system_clock::now();

        auto now_time_t = std::chrono::system_clock::to_time_t(now);

        std::tm now_tm = *std::localtime(&now_time_t);

        std::stringstream ss;
        ss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");

        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                          now.time_since_epoch()) %
                      1000;
        ss << "." << std::setfill('0') << std::setw(3) << now_ms.count();

        return ss.str();
    }
};

#endif // LOGINFOMODEL_H
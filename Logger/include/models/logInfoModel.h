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
        // 使用 std::chrono 獲取當前時間點
        auto now = std::chrono::system_clock::now();

        // 轉換為時間_t格式
        auto now_time_t = std::chrono::system_clock::to_time_t(now);

        // 轉換為tm結構以方便格式化
        std::tm now_tm = *std::localtime(&now_time_t);

        // 使用 stringstream 進行格式化
        std::stringstream ss;
        ss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");

        // Add milliseconds
        auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                          now.time_since_epoch()) %
                      1000;
        ss << "." << std::setfill('0') << std::setw(3) << now_ms.count();
        // 返回格式化後的字符串
        return ss.str();
    }
};

#endif // LOGINFOMODEL_H
#ifndef MESSAGEFORMATTER_H
#define MESSAGEFORMATTER_H
#include <string>
#include "models/logInfoModel.h"
#include "models/sysInfoModel.h"

class MessageFormatter
{
public:
    virtual ~MessageFormatter() = default;
    virtual std::string format(sysInfoModel sysInfo, logInfoModel loginfo) = 0; 
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

#endif // MESSAGE_FORMATTER_H

#ifndef MESSAGEFORMATTER_H
#define MESSAGEFORMATTER_H
#include <string>
#include "models/logInfoModel.h"
#include "models/sysInfoModel.h"

class MessageFormatter
{
public:
    virtual ~MessageFormatter() = default;
    virtual std::string format(sysInfoModel sysInfo, logInfoModel loginfo) = 0; // 處理訊息格式化的虛擬方法
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

#endif // MESSAGE_FORMATTER_H

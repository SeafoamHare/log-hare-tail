#ifndef LOG_HARE_TAIL_FORMAT_JSON_MESSAGE_FORMATTER_H
#define LOG_HARE_TAIL_FORMAT_JSON_MESSAGE_FORMATTER_H
#include <string>

#include <nlohmann/json.hpp>

#include "log_hare_tail/format/message_formatter.h"

using json = nlohmann::json;

class JsonMessageFormatter : public MessageFormatter
{
public:
    std::string format(const sysInfoModel &sysInfo, const logInfoModel &loginfo) override;
};

#endif // LOG_HARE_TAIL_FORMAT_JSON_MESSAGE_FORMATTER_H

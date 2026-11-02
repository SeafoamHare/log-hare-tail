#ifndef LOG_HARE_TAIL_FORMAT_PLAIN_MESSAGE_FORMATTER_H
#define LOG_HARE_TAIL_FORMAT_PLAIN_MESSAGE_FORMATTER_H

#include <string>

#include "log_hare_tail/format/message_formatter.h"

class PlainMessageFormatter : public MessageFormatter
{
public:
    std::string format(const sysInfoModel &sysInfo, const logInfoModel &loginfo) override;
};

#endif // LOG_HARE_TAIL_FORMAT_PLAIN_MESSAGE_FORMATTER_H

#ifndef PLAINMESSAGEFORMATTER_H
#define PLAINMESSAGEFORMATTER_H

#include <string>
#include "MessageFormatter.h"

class PlainMessageFormatter : public MessageFormatter
{
public:
    std::string format(const sysInfoModel &sysInfo, const logInfoModel &loginfo) override;
};

#endif // PLAINMESSAGEFORMATTER_H
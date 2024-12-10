#ifndef PLAINMESSAGEFORMATTER_H
#define PLAINMESSAGEFORMATTER_H

#include <string>
#include "MessageFormatter.h"

class PlainMessageFormatter : MessageFormatter
{
public:
    std::string format(sysInfoModel sysInfo, logInfoModel loginfo) override;
};

#endif // PLAINMESSAGEFORMATTER_H
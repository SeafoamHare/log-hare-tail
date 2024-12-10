#ifndef JSONMESSAGEFORMATTER_H
#define JSONMESSAGEFORMATTER_H
#include <nlohmann/json.hpp>
#include <string>
#include "MessageFormatter.h"

using json = nlohmann::json;

class JsonMessageFormatter : public MessageFormatter
{
public:
    std::string format(sysInfoModel sysInfo, logInfoModel loginfo) override;
};

#endif // JSONMESSAGEFORMATTER_H
#include "formatMessage/JsonMessageFormatter.h"

std::string JsonMessageFormatter::format(sysInfoModel sysInfo, logInfoModel loginfo)
{
    // to do: Remove dependency on JsonMessageFormatter
    // std::ostringstream oss;
    // oss << "{";
    // oss << "\"@timestamp\":\"" << getCurrentTimestamp() << "\",";
    // oss << "\"log.logger\":\"" << logger_name << "\",";
    // oss << "\"log.level\":\"" << levelToString(level) << "\",";
    // oss << "\"message.context\":\"" << message << "\",";
    // oss << "\"app.name\":\"" << m_config->app_name << "\",";
    // oss << "\"file\":\"" << file << "\",";
    // oss << "\"line\":" << line;
    // oss << "}";
    // return oss.str();
    json j;
    j["@timestamp"] = getCurrentTimestamp();
    j["log.logger"] = loginfo.function;
    j["log.level"] = loglevelToString(loginfo.level);
    j["message.context"] = loginfo.message;
    j["app.name"] = sysInfo.app_name;
    j["file"] = loginfo.file;
    j["line"] = loginfo.line;
    if (!loginfo.device_id.empty())
    {
        j["device.id"] = loginfo.device_id;
    }
    j["pod.id"] = sysInfo.pod_id;
    return j.dump();
}

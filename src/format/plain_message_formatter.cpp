#include "log_hare_tail/format/plain_message_formatter.h"

std::string PlainMessageFormatter::format(const sysInfoModel &sysInfo, const logInfoModel &loginfo)
{
    std::ostringstream oss;
    oss << getCurrentTimestamp() << " ";
    oss << "[" << loglevelToString(loginfo.level) << "] ";
    oss << "[" << loginfo.function << "] ";
    oss << loginfo.message << " ";
    oss << "(file: " << loginfo.file << ", line: " << loginfo.line << ") ";
    oss << "(app: " << sysInfo.app_name << ") ";
    oss << "(pod.id: " << sysInfo.pod_id << ")";
    if (!loginfo.device_id.empty())
    {
        oss << " (device: " << loginfo.device_id << ")";
    }
    return oss.str();
}

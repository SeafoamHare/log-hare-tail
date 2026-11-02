#ifndef LOG_HARE_TAIL_LOG_LEVEL_H
#define LOG_HARE_TAIL_LOG_LEVEL_H

#include <cstdint>
#include <string>

enum class LogLevel : std::uint8_t
{
    DEBUG,
    INFO,
    WARN,
    ERROR,
    CRITICAL
};

std::string loglevelToString(LogLevel level);

#endif // LOG_HARE_TAIL_LOG_LEVEL_H

// Level.h
#ifndef LEVEL_H
#define LEVEL_H

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

#endif // LEVEL_H

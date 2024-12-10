// Level.h
#ifndef LEVEL_H
#define LEVEL_H

#include <string>

enum class LogLevel
{
    DEBUG,
    INFO,
    WARN,
    ERROR,
    CRITICAL
};

std::string loglevelToString(LogLevel level);

#endif // LEVEL_H

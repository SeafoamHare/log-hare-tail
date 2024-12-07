// Level.cpp
#include "Level.h"

// 实现转换函数
std::string levelToString(Level level)
{
    switch (level) {
        case Level::DEBUG:    return "DEBUG";
        case Level::INFO:     return "INFO";
        case Level::WARN:     return "WARN";
        case Level::ERROR:    return "ERROR";
        case Level::CRITICAL: return "CRITICAL";
        default:              return "UNKNOWN";
    }
}

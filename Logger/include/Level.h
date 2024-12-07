// Level.h
#ifndef LEVEL_H
#define LEVEL_H

#include <string>

enum class Level
{
    DEBUG,
    INFO,
    WARN,
    ERROR,
    CRITICAL
};

// 转换函数的声明
std::string levelToString(Level level);

#endif // LEVEL_H

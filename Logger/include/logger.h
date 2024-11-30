#ifndef LOGGER_H
#define LOGGER_H

#include <ctime>
#include <iomanip>
#include <filesystem>
#include <sstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <fstream>
#include <stdexcept>
#include <sys/stat.h>
#include <errno.h>
#include <cstring>
#include <thread>
#include <mutex>
#include <vector>
#include "podInfo_Retriever.h"
#include "config_file.h"

using json = nlohmann::json;
// 只在 C++11 環境下定義 make_unique
#if __cplusplus == 201103L
namespace std
{
    template <typename T, typename... Args>
    typename std::enable_if<!std::is_array<T>::value, std::unique_ptr<T>>::type
    make_unique(Args &&...args)
    {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }

    template <typename T>
    typename std::enable_if<std::is_array<T>::value && std::extent<T>::value == 0, std::unique_ptr<T>>::type
    make_unique(size_t size)
    {
        typedef typename std::remove_extent<T>::type U;
        return std::unique_ptr<T>(new U[size]());
    }

    template <typename T, typename... Args>
    typename std::enable_if<std::extent<T>::value != 0, void>::type
    make_unique(Args &&...) = delete;
}
#endif
#define LOG_DEBUG(device_id)                                                           \
    Logger::getInstance() << Logger::Level::DEBUG << Logger::LogFunction(__FUNCTION__) \
                          << Logger::LogFile(__FILE__) << Logger::LogLine(__LINE__)    \
                          << Logger::DeviceId(device_id)
#define LOG_INFO(device_id)                                                           \
    Logger::getInstance() << Logger::Level::INFO << Logger::LogFunction(__FUNCTION__) \
                          << Logger::LogFile(__FILE__) << Logger::LogLine(__LINE__)   \
                          << Logger::DeviceId(device_id)
#define LOG_WARN(device_id)                                                           \
    Logger::getInstance() << Logger::Level::WARN << Logger::LogFunction(__FUNCTION__) \
                          << Logger::LogFile(__FILE__) << Logger::LogLine(__LINE__)   \
                          << Logger::DeviceId(device_id)
#define LOG_ERROR(device_id)                                                           \
    Logger::getInstance() << Logger::Level::ERROR << Logger::LogFunction(__FUNCTION__) \
                          << Logger::LogFile(__FILE__) << Logger::LogLine(__LINE__)    \
                          << Logger::DeviceId(device_id)
#define LOG_CRITICAL(device_id)                                                           \
    Logger::getInstance() << Logger::Level::CRITICAL << Logger::LogFunction(__FUNCTION__) \
                          << Logger::LogFile(__FILE__) << Logger::LogLine(__LINE__)       \
                          << Logger::DeviceId(device_id)

class Logger
{
public:
    enum class Level
    {
        DEBUG,
        INFO,
        WARN,
        ERROR,
        CRITICAL
    };

    static Logger &getInstance()
    {
        static Logger instance;
        return instance;
    }

    void initialize(const std::string &config_file);
    void setLevel(Level level);
    void log(Level level, const std::string &logger_name, const std::string &file, int line);
    // Delete copy constructor and assignment operator
    Logger(Logger const &) = delete;
    void operator=(Logger const &) = delete;

    // 用包裝結構來定義特定 __FUNCTION__, __FILE__, __LINE__
    struct LogLine
    {
        int value;
        explicit LogLine(int line) : value(line) {}
    };

    struct LogFunction
    {
        const char *value;
        explicit LogFunction(const char *function) : value(function) {}
    };

    struct LogFile
    {
        const char *value;
        explicit LogFile(const char *file) : value(file) {}
    };

    struct DeviceId
    {
        std::string value;
        DeviceId() : value("") {} // 添加默認構造函數
        explicit DeviceId(const std::string &id) : value(id) {}
    };

    // 重载输出流操作符
    template <typename T>
    Logger &operator<<(const T &value)
    {
        _stream << value;
        return *this;
    }

    Logger &operator<<(Level level)
    {
        _level = level;
        return *this;
    }

    Logger &operator<<(LogLine line)
    {
        _line = line.value;
        return *this;
    }

    Logger &operator<<(LogFunction function)
    {
        _function = function.value;
        return *this;
    }

    Logger &operator<<(LogFile file)
    {
        _local_file = file.value;
        return *this;
    }

    Logger &operator<<(DeviceId d_id)
    {
        _device_id = d_id.value;
        return *this;
    }

    // 结束符：输出流结束时调用log()
    Logger &operator<<(std::ostream &(*endl)(std::ostream &))
    {
        log(_level, _function, _local_file, _line);
        _stream.str(""); // 清空流
        return *this;
    }

private:
    Logger() = default;
    ~Logger();

    std::unique_ptr<Config_File> m_config;
    std::ofstream m_file;
    std::string m_full_path;
    std::string m_pod_id;
    std::mutex m_mutex;
    thread_local static std::ostringstream _stream;
    thread_local static Level _level;
    thread_local static std::string _function;
    thread_local static std::string _local_file;
    thread_local static int _line;
    thread_local static std::string _device_id;
    void rotate();
    std::string levelToString(Level level);
    std::string getCurrentTimestamp();
    std::string formatJsonMessage(Level level, const std::string &message, const std::string &logger_name, const std::string &file, int line);
    std::string formatPlainMessage(Level level, const std::string &message, const std::string &logger_name, const std::string &file, int line);
};

#endif // LOGGER_H
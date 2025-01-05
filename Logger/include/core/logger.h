#ifndef LOGGER_H
#define LOGGER_H

#include <mutex>
#include "podInfo_Retriever.h"
#include "config_file.h"
#include "LogLevel.h"
#include "models/logInfoModel.h"
#include "models/sysInfoModel.h"
#include "formatMessage/JsonMessageFormatter.h"
#include "formatMessage/MessageFormatter.h"
#include "formatMessage/PlainMessageFormatter.h"
#include "log_store_controller.h"

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
#define LOG_DEBUG(device_id)                                                        \
    Logger::getInstance() << LogLevel::DEBUG << Logger::LogFunction(__FUNCTION__)   \
                          << Logger::LogFile(__FILE__) << Logger::LogLine(__LINE__) \
                          << Logger::DeviceId(device_id)
#define LOG_INFO(device_id)                                                         \
    Logger::getInstance() << LogLevel::INFO << Logger::LogFunction(__FUNCTION__)    \
                          << Logger::LogFile(__FILE__) << Logger::LogLine(__LINE__) \
                          << Logger::DeviceId(device_id)
#define LOG_WARN(device_id)                                                         \
    Logger::getInstance() << LogLevel::WARN << Logger::LogFunction(__FUNCTION__)    \
                          << Logger::LogFile(__FILE__) << Logger::LogLine(__LINE__) \
                          << Logger::DeviceId(device_id)
#define LOG_ERROR(device_id)                                                        \
    Logger::getInstance() << LogLevel::ERROR << Logger::LogFunction(__FUNCTION__)   \
                          << Logger::LogFile(__FILE__) << Logger::LogLine(__LINE__) \
                          << Logger::DeviceId(device_id)
#define LOG_CRITICAL(device_id)                                                      \
    Logger::getInstance() << LogLevel::CRITICAL << Logger::LogFunction(__FUNCTION__) \
                          << Logger::LogFile(__FILE__) << Logger::LogLine(__LINE__)  \
                          << Logger::DeviceId(device_id)

class Logger
{
public:
    static Logger &getInstance()
    {
        static Logger instance;
        return instance;
    }

    void initialize(const std::string &config_file);

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

    Logger &operator<<(LogLevel level)
    {
        _logInfo.level = level;
        return *this;
    }

    Logger &operator<<(LogLine line)
    {
        _logInfo.line = line.value;
        return *this;
    }

    Logger &operator<<(LogFunction function)
    {
        _logInfo.function = function.value;
        return *this;
    }

    Logger &operator<<(LogFile file)
    {
        _logInfo.file = file.value;
        return *this;
    }

    Logger &operator<<(DeviceId d_id)
    {
        _logInfo.device_id = d_id.value;
        return *this;
    }

    // 结束符：输出流结束时调用log()
    Logger &operator<<(std::ostream &(*endl)(std::ostream &))
    {
        log();
        _stream.str(""); // 清空流
        return *this;
    }

private:
    Logger() = default;
    ~Logger();
    void log();
    void setMessageFormatter(std::shared_ptr<MessageFormatter> &&formate);
    std::string formatMessage() const;

    std::unique_ptr<Config_File> m_config;
    std::shared_ptr<MessageFormatter> _formatter;
    std::mutex m_mutex;
    thread_local static std::ostringstream _stream;
    thread_local static logInfoModel _logInfo;
    sysInfoModel sysInfo_;
    std::unique_ptr<LogStoreController> logStoreController;
    // void rotate();
};

#endif // LOGGER_H
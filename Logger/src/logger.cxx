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
#include "PodInfoRetriever.cxx"
#include "config_file.cxx"

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
                          << Logger::DeviceId(device_id.)
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

void Logger::initialize(const std::string &config_file)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_pod_id = PodInfoRetriever::getPodId();
    m_config = std::make_unique<Config_File>(config_file);
    m_full_path = m_config->log_directory + "/" + m_config->filename;
    m_file.open(m_full_path, std::ios::app);
    if (!m_file.is_open())
    {
        throw std::runtime_error("Unable to open log file: " + m_full_path);
    }
}

void Logger::log(Level level, const std::string &logger_name, const std::string &file, int line)
{

    std::string message = _stream.str();
    // JSON format for console output
    std::string json_message = formatJsonMessage(level, message, logger_name, file, line);
    std::cout << json_message << std::endl;
    std::lock_guard<std::mutex> lock(m_mutex);
    // Plain text format for file output
    if (m_file.is_open())
    {
        std::string plain_message = formatPlainMessage(level, message, logger_name, file, line);
        m_file << plain_message << std::endl;
        m_file.flush();

        // Check file size and rotate if necessary
        m_file.seekp(0, std::ios::end);
        if (static_cast<size_t>(m_file.tellp()) >= m_config->max_size)
        {
            rotate();
        }
    }
}

Logger::~Logger()
{
    if (m_file.is_open())
    {
        m_file.close();
    }
}

std::string Logger::getCurrentTimestamp()
{
    // 使用 std::chrono 獲取當前時間點
    auto now = std::chrono::system_clock::now();

    // 轉換為時間_t格式
    auto now_time_t = std::chrono::system_clock::to_time_t(now);

    // 轉換為tm結構以方便格式化
    std::tm now_tm = *std::localtime(&now_time_t);

    // 使用 stringstream 進行格式化
    std::stringstream ss;
    ss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");

    // Add milliseconds
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                      now.time_since_epoch()) %
                  1000;
    ss << "." << std::setfill('0') << std::setw(3) << now_ms.count();
    // 返回格式化後的字符串
    return ss.str();
}

std::string Logger::levelToString(Level level)
{
    switch (level)
    {
    case Level::INFO:
        return "INFO";
    case Level::WARN:
        return "WARN";
    case Level::ERROR:
        return "ERROR";
    case Level::CRITICAL:
        return "CRITICCAL";
    case Level::DEBUG:
        return "DEBUG";
    default:
        return "UNKNOWN";
    }
}

std::string Logger::formatJsonMessage(Level level, const std::string &message, const std::string &logger_name, const std::string &file, int line)
{
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
    j["log.logger"] = logger_name;
    j["log.level"] = levelToString(level);
    j["message.context"] = message;
    j["app.name"] = m_config->app_name;
    j["file"] = file;
    j["line"] = line;
    if (!_device_id.empty())
    {
        j["device.id"] = _device_id;
    }
    j["pod.id"] = m_pod_id;
    return j.dump();
}

std::string Logger::formatPlainMessage(Level level, const std::string &message, const std::string &logger_name, const std::string &file, int line)
{
    std::ostringstream oss;
    oss << getCurrentTimestamp() << " ";
    oss << "[" << levelToString(level) << "] ";
    oss << "[" << logger_name << "] ";
    oss << message << " ";
    oss << "(file: " << file << ", line: " << line << ") ";
    oss << "(app: " << m_config->app_name << ") ";
    oss << "(pod.id: " << m_pod_id << ")";
    if (!_device_id.empty())
    {
        oss << " (device: " << _device_id << ")";
    }
    return oss.str();
}

void Logger::rotate()
{
    m_file.close();

    // Rename existing log files
    for (int i = m_config->max_files - 2; i > 0; --i)
    {
        std::string old_name = m_config->log_directory + "/" + std::to_string(i - 1) + "." + m_config->filename;
        std::string new_name = m_config->log_directory + "/" + std::to_string(i) + "." + m_config->filename;
        rename(old_name.c_str(), new_name.c_str());
    }

    // Rename the current log file
    // std::string backup_name = "0." + m_full_path;
    std::string backup_name = m_config->log_directory + "/0." + m_config->filename;
    rename(m_full_path.c_str(), backup_name.c_str());

    // Open a new log file
    m_file.open(m_full_path, std::ios::app);
    if (!m_file.is_open())
    {
        throw std::runtime_error("Unable to open log file: " + m_full_path);
    }
}

// 初始化 thread_local 變量
thread_local std::ostringstream Logger::_stream;
thread_local Logger::Level Logger::_level = Logger::Level::DEBUG;
thread_local std::string Logger::_function;
thread_local std::string Logger::_local_file;
thread_local int Logger::_line = 0;
thread_local std::string Logger::_device_id;

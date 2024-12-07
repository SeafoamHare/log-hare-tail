#include "../include/logger.h"

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
thread_local Level Logger::_level = Level::DEBUG;
thread_local std::string Logger::_function;
thread_local std::string Logger::_local_file;
thread_local int Logger::_line = 0;
thread_local std::string Logger::_device_id;

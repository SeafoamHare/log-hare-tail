#include "core/logger.h"

void Logger::initialize(const std::string &config_file)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    m_config = std::make_unique<Config_File>(config_file);
    m_full_path = m_config->log_directory + "/" + m_config->filename;
    m_file.open(m_full_path, std::ios::app);
    if (!m_file.is_open())
    {
        throw std::runtime_error("Unable to open log file: " + m_full_path);
    }
    _sysInfo.app_name = m_config->app_name;
    _sysInfo.pod_id = PodInfoRetriever::getPodId();
}

void Logger::log()
{

    std::string message = _stream.str();
    // JSON format for console output
    std::lock_guard<std::mutex> lock(m_mutex);
    setMessageFormatter(std::make_shared<PlainMessageFormatter>());
    std::string json_message = formatMessage();
    std::cout << json_message << std::endl;

    // Plain text format for file output
    if (m_file.is_open())
    {
        setMessageFormatter(std::make_shared<JsonMessageFormatter>());
        std::string plain_message = formatMessage();
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

void Logger::setMessageFormatter(std::shared_ptr<MessageFormatter> &&formatter)
{
    this->_formatter = std::move(formatter);
}

std::string Logger::formatMessage() const
{
    if (_formatter)
    {
        return _formatter->format(_sysInfo, _logInfo);
    }
    else
    {
        return "No formatter set!";
    }
}

Logger::~Logger()
{
    if (m_file.is_open())
    {
        m_file.close();
    }
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
thread_local logInfoModel Logger::_logInfo;

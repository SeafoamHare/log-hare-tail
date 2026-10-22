#include "core/logger.h"
#include <stdexcept>

void Logger::initialize(const std::string &config_file)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (logStoreController)
    {
        throw std::logic_error("Logger::initialize() called twice");
    }

    m_config = std::make_unique<Config_File>(config_file);
    logStoreController = std::make_unique<LogStoreController>(*m_config);
    sysInfo_.app_name = m_config->app_name;
    sysInfo_.pod_id = PodInfoRetriever::getPodId();
}

void Logger::log()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    _logInfo.message = _stream.str();
    _stream.str("");
    _stream.clear();
    std::string json_message = console_formatter_->format(sysInfo_, _logInfo);
    std::cout << json_message << std::endl;

    if (!logStoreController)
    {
        if (!warned_uninitialized_)
        {
            std::cerr << "WARNING: Logging before Logger::initialize() is written to console only" << std::endl;
            warned_uninitialized_ = true;
        }
        return;
    }

    // Plain text format for file output
    std::string plain_message = file_formatter_->format(sysInfo_, _logInfo);
    logStoreController->Write(plain_message);
}

Logger::~Logger()
{
}

thread_local std::ostringstream Logger::_stream;
thread_local logInfoModel Logger::_logInfo;

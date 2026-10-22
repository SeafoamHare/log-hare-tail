#include "core/logger.h"
#include <stdexcept>

void Logger::initialize(const std::string &config_file)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    // Same as glog's "You called InitGoogleLogging() twice!": a second call would silently
    // swap the log file under running threads, so treat it as a programming error.
    // logStoreController is only set once everything below succeeded, so a failed call can be retried
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
    // _stream is thread_local and lives as long as the thread, so reset it for the next message
    _stream.str("");
    _stream.clear();
    // JSON format for console output
    std::string json_message = console_formatter_->format(sysInfo_, _logInfo);
    std::cout << json_message << std::endl;

    // Like glog, logging before initialize() degrades to console only instead of crashing,
    // and warns once on stderr so the missing call is still noticed
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

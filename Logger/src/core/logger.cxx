#include "core/logger.h"

void Logger::initialize(const std::string &config_file)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    m_config = std::make_unique<Config_File>(config_file);
    logStoreController = std::make_unique<LogStoreController>(*m_config);
    sysInfo_.app_name = m_config->app_name;
    sysInfo_.pod_id = PodInfoRetriever::getPodId();
}

void Logger::log()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    _logInfo.message = _stream.str();
    setMessageFormatter(std::make_shared<PlainMessageFormatter>());
    std::string plain_message = formatMessage();
    std::cout << plain_message << std::endl;
    setMessageFormatter(std::make_shared<JsonMessageFormatter>());
    std::string json_message = formatMessage();
    logStoreController->Write(json_message);
}

void Logger::setMessageFormatter(std::shared_ptr<MessageFormatter> &&formatter)
{
    this->_formatter = std::move(formatter);
}

std::string Logger::formatMessage() const
{
    if (_formatter)
    {
        return _formatter->format(sysInfo_, _logInfo);
    }
    else
    {
        return "No formatter set!";
    }
}

Logger::~Logger()
{
}

thread_local std::ostringstream Logger::_stream;
thread_local logInfoModel Logger::_logInfo;

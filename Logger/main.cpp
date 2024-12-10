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
// #include "logMessage.cpp"
#include "include/core/logger.h"
void workerFunction(int id)
{
    for (int i = 0; i < 10; ++i)
    {
        LOG_INFO() << std::to_string(id) << " This is a info message " << std::to_string(i) << std::endl;
        LOG_WARN() << std::to_string(id) << " This is a info message " << std::to_string(i) << std::endl;
        LOG_ERROR() << std::to_string(id) << " This is a warning message " << std::to_string(i) << std::endl;
        LOG_CRITICAL() << std::to_string(id) << " This is a error message " << std::to_string(i) << std::endl;
        LOG_DEBUG() << std::to_string(id) << " This is a debug message " << std::to_string(i) << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        // logger.log("Worker " + std::to_string(id) + " - Task " + std::to_string(i));
    }
}
int main()
{
    try
    {
        Logger::getInstance().initialize("logger_config.json");
        // LOG_INFO("Application started");
        LOG_INFO("DEVICE-001") << "Application started" << std::endl;
        std::vector<std::thread> threads;
        const int numThreads = 5;

        for (int i = 0; i < numThreads; ++i)
        {
            threads.emplace_back(workerFunction, i);
        }

        for (auto &thread : threads)
        {
            thread.join();
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

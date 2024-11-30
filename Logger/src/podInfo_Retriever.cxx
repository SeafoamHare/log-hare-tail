#include "../include/podInfo_Retriever.h"

std::string PodInfoRetriever::getPodId()
{
    char hostname[HOST_NAME_MAX];
    if (gethostname(hostname, HOST_NAME_MAX) != 0)
    {
        throw std::runtime_error("Failed to get hostname");
    }
    std::cout << std::string(hostname) << std::endl;
    return std::string(hostname);
}

// 從環境變數獲取 Pod IP
std::string PodInfoRetriever::getPodIp()
{
    const char *podIp = std::getenv("POD_IP");
    if (podIp == nullptr)
    {
        throw std::runtime_error("Failed to get Pod IP");
    }
    return std::string(podIp);
}

// 從環境變數獲取命名空間
std::string PodInfoRetriever::getNamespace()
{
    const char *ns = std::getenv("POD_NAMESPACE");
    if (ns == nullptr)
    {
        throw std::runtime_error("Failed to get namespace");
    }
    return std::string(ns);
}
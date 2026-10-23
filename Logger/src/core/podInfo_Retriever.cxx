#include "core/podInfo_Retriever.h"

// HOST_NAME_MAX is Linux-only (macOS does not define it); POSIX guarantees hostnames up to 255 bytes
#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif

std::string PodInfoRetriever::getPodId()
{
    // gethostname() does not have to null-terminate a truncated name, so keep one byte spare
    char hostname[HOST_NAME_MAX + 1] = {};
    if (gethostname(hostname, HOST_NAME_MAX) != 0)
    {
        throw std::runtime_error("Failed to get hostname");
    }
    return std::string(hostname);
}

std::string PodInfoRetriever::getPodIp()
{
    const char *podIp = std::getenv("POD_IP");
    if (podIp == nullptr)
    {
        throw std::runtime_error("Failed to get Pod IP");
    }
    return std::string(podIp);
}

std::string PodInfoRetriever::getNamespace()
{
    const char *ns = std::getenv("POD_NAMESPACE");
    if (ns == nullptr)
    {
        throw std::runtime_error("Failed to get namespace");
    }
    return std::string(ns);
}
#ifndef PODINFO_RETRIEVER_H
#define PODINFO_RETRIEVER_H

#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <limits.h>
#include <stdexcept>

class PodInfoRetriever
{
public:
    // 獲取 Pod ID
    static std::string getPodId();

    // 從環境變數獲取 Pod IP
    static std::string getPodIp();

    // 從環境變數獲取命名空間
    static std::string getNamespace();
};

#endif // PODINFO_RETRIEVER_H

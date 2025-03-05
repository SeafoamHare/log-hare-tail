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
    static std::string getPodId();

    static std::string getPodIp();

    static std::string getNamespace();
};

#endif // PODINFO_RETRIEVER_H

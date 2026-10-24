#ifndef PODINFO_RETRIEVER_H
#define PODINFO_RETRIEVER_H

#include <limits.h>
#include <unistd.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

class PodInfoRetriever
{
public:
    static std::string getPodId();

    static std::string getPodIp();

    static std::string getNamespace();
};

#endif // PODINFO_RETRIEVER_H

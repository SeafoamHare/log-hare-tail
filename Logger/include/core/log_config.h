#include <string>
#include <iostream>

class LogConfig
{
public:
    static LogConfig &getInstance()
    {
        static LogConfig instance;
        return instance;
    }

    void initialize(const std::string &app_name, const std::string &device_id, const std::string &pod_id)
    {
        this->app_name = app_name;
        this->pod_id = pod_id;
    }

    const std::string &getAppName() const { return app_name; }
    const std::string &getDeviceId() const { return device_id; }
    const std::string &getPodId() const { return pod_id; }

private:
    LogConfig() = default;

    std::string app_name;
    std::string device_id;
    std::string pod_id;
};

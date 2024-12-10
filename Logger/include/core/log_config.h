#include <string>
#include <iostream>

class LogConfig {
public:
    // 单例模式，确保 LogConfig 只会初始化一次
    static LogConfig& getInstance() {
        static LogConfig instance;
        return instance;
    }

    // 设置一些不变的信息
    void initialize(const std::string& app_name, const std::string& device_id, const std::string& pod_id) {
        this->app_name = app_name;
        this->pod_id = pod_id;
    }

    // 获取不变的信息
    const std::string& getAppName() const { return app_name; }
    const std::string& getDeviceId() const { return device_id; }
    const std::string& getPodId() const { return pod_id; }

private:
    // 私有化构造函数，避免外部直接创建
    LogConfig() = default;

    // 不变的配置项
    std::string app_name;
    std::string device_id;
    std::string pod_id;
};

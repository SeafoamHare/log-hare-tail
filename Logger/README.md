# C++ Logger System

A thread-safe, feature-rich logging system for C++ applications with JSON console output and plain text file output capabilities.

## Features

- **Dual Output Format**
  - Console: Structured JSON format for easy parsing and analysis
  - File: Human-readable plain text format
  
- **Comprehensive Logging Levels**
  - DEBUG
  - INFO
  - WARN
  - ERROR
  - CRITICAL

- **Automatic Log Rotation**
  - Size-based rotation
  - Configurable maximum file size
  - Configurable number of backup files
  - Automatic file management

- **Thread Safety**
  - Mutex-protected operations
  - Safe for multi-threaded applications

- **Flexible Configuration**
  - JSON-based configuration file
  - Runtime configuration loading
  - Customizable log directory and file names

- **Rich Metadata**
  - Timestamp with millisecond precision
  - Source file and line number
  - Function name
  - Application name

## Requirements
- C++11 or higher (for thread_local support and other modern C++ features)
- nlohmann/json library

```
$ sudo apt-get update
$ sudo apt-get install libjsoncpp-dev
```

## Configuration
Create a `config.json` file with the following structure:

```json
{
    "log_directory": "/var/log/myapp",
    "filename": "app.log",
    "max_size": 10485760,
    "max_files": 5,
    "app_name": "MyApplication",
}
```

### Configuration Parameters

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| log_directory | string | Yes | Directory where log files will be stored |
| filename | string | Yes | Name of the log file |
| max_size | number | Yes | Maximum size of each log file in bytes |
| max_files | number | Yes | Maximum number of backup files to keep |
| app_name | string | Yes | Name of the application |


## Usage
### Format with/without Device ID

Log messages can be written with or without a device ID:
```cpp
// With device ID
LOG_INFO("DEVICE-001") << "Application started" << std::endl;

// Without device ID
LOG_INFO() << "Application started" << std::endl;
```
### Basic Usage

```cpp
#include "/Logger/src/logger.cxx"

int main() {
    try {
        // Initialize the logger
        Logger::getInstance().initialize("config.json");
        LOG_INFO("DEVICE-001","POD-ID") << "Application started" << std::endl;

        // Log messages
        LOG_INFO("DEVICE-001","POD-ID") << "This is a info message" << std::endl;
        LOG_WARN("DEVICE-001","POD-ID") << "This is a warning message" << std::endl;
        LOG_ERROR("DEVICE-001","POD-ID") << "This is a error message" << std::endl;
        LOG_CRITICAL("DEVICE-001","POD-ID") << "This is a critical message" << std::endl;
        LOG_DEBUG("DEVICE-001","POD-ID") << "This is a debug message" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

### Output Format ( With device id )

#### Console Output (JSON)
```json
{
  "@timestamp":"2024-11-08 18:00:27.884",
  "app.name":"MyApplication",
  "file":"main.cpp",
  "line":36,
  "log.level":"INFO",
  "log.logger":"main",
  "message.context":"Application started",
  "pod.id":"hare-ThinkPad-E16-Gen-2"
}
```

#### File Output (Plain Text)
```
2024-10-30T14:30:45.123 [INFO] [main] Application started (file: main.cpp, line: 10) (app: MyApplication) (device: DEVICE-001) (pod.id: device-state-provider-6c58ffdbb8-f2m6m)
```
### Output Format ( Without device id )

#### Console Output (JSON)
```json
{
  "@timestamp":"2024-10-30T14:30:45.123",
  "app.name":"MyApplication",
  "file":"main.cpp",
  "line":36,
  "log.level":"INFO",
  "log.logger":"main",
  "message.context":"Application started",
  "pod.id":"hare-ThinkPad-E16-Gen-2"
}
```

#### File Output (Plain Text)
```
2024-10-30T14:30:45.123 [INFO] [main] Application started (file: main.cpp, line: 10) (app: MyApplication) (pod.id: device-state-provider-6c58ffdbb8-f2m6m)
```
## Log Rotation

The logger automatically rotates log files when they reach the configured maximum size. The rotation process:

1. Renames existing backup files by incrementing their number
2. Moves the current log file to .1
3. Creates a new log file

Example of rotated files:
```
app.log      # Current log file
1.app.log    # Most recent backup
2.app.log    # Second most recent backup
3.app.log    # Third most recent backup
```

## Thread Safety

The logger is designed to be thread-safe:
- Critical parameters (file paths, log levels, logger names, etc.) are isolated per thread using thread-local storage
- File operations are protected by a mutex
- Configuration is thread-safe after initialization

## Error Handling

The logger uses exceptions to handle errors:
- Configuration errors (invalid JSON, missing required fields)
- File system errors (permission denied, disk full)
- Runtime errors

## Best Practices

1. Initialize the logger early in your application
2. Use appropriate log levels:
   - DEBUG: Detailed information for debugging
   - INFO: General operational messages
   - WARN: Warning messages for potential issues
   - ERROR: Error messages for actual problems
   - CRITICAL: Critical errors that require immediate attention

3. Include relevant context in log messages
4. Monitor log rotation to ensure adequate disk space
5. Configure appropriate max_size and max_files for your use case
## Directory Structure
```
Logger/
├── src/              # Source files
│   ├── logger/       # Core functionality
│   ├── config_file/  # Additional modules
│   └── podInfo_Retriever.cxx/  # Get PODid
```
## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
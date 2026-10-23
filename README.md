# C++ Logger System

[![CI](https://github.com/SeafoamHare/log-hare-tail/actions/workflows/ci.yml/badge.svg)](https://github.com/SeafoamHare/log-hare-tail/actions/workflows/ci.yml)

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
  - Configurable total number of log files
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
  - Device ID (optional)
  - Pod ID (the hostname, which is the pod name when running in Kubernetes)

## Requirements
- A C++11 compiler for the library (the tests use C++17). CI builds the library as C++11 to keep this true
- CMake 3.25 or newer, and Ninja
- nlohmann/json library

```
# Ubuntu / Debian
$ sudo apt-get update
$ sudo apt-get install cmake ninja-build nlohmann-json3-dev

# macOS
$ brew install cmake ninja nlohmann-json
```

## Build & Run

Builds are driven by [CMake presets](Logger/CMakePresets.json); each preset builds into `Logger/build/<OS>/<preset>/`.

```
$ cd Logger
$ cmake --workflow --preset dev       # configure, build and run the tests
$ ./build/Darwin/dev/logimp           # run the demo (build/Linux/dev on Linux)
```

Or build and run inside Docker:

```
$ docker build -t log-hare-tail .
$ docker run --rm log-hare-tail
```

## Testing

Tests use [GoogleTest](https://github.com/google/googletest) v1.14.0, which CMake downloads on the first configure.
CTest runs every test case in its own process, in parallel.

| Preset | What it runs |
|---|---|
| `dev` | Unit tests with your default compiler |
| `gcc`, `clang` | Unit tests with that compiler, warnings treated as errors |
| `tsan` | Unit tests under ThreadSanitizer (data races) |
| `asan` | Unit tests under AddressSanitizer + UndefinedBehaviorSanitizer |
| `release` | Optimized demo build without tests (used by the Docker image) |

```
$ cd Logger
$ cmake --workflow --preset asan                       # configure + build + test in one step
$ ctest --preset dev -R LoggerStreamTest               # rerun only the tests matching a name
```

### Running the CI checks locally

CI runs every Linux check inside the `dev` stage of the Dockerfile, with the checkout mounted in.
The same commands reproduce a CI failure on your machine:

```
$ docker build --target dev -t log-hare-tail:dev .
$ docker run --rm -v "$PWD:/workspace" log-hare-tail:dev cmake --workflow --preset gcc
$ docker run --rm -v "$PWD:/workspace" log-hare-tail:dev cmake --workflow --preset asan
```

Linux builds land in `build/Linux/`, so they never mix with builds made on a macOS host.

| CI job | What it checks |
|---|---|
| `docker build` | Every Dockerfile stage builds, and the project compiles on Linux |
| `linux / gcc`, `linux / clang` | Unit tests pass, with warnings treated as errors |
| `linux / tsan` | No data races under ThreadSanitizer |
| `linux / asan` | No memory errors, leaks, or undefined behavior under ASan + UBSan |
| `macos / clang` | Builds and passes tests natively on macOS |

## Configuration
Create a `config.json` file with the following structure:

```json
{
    "log_directory": "/var/log/myapp",
    "filename": "app.log",
    "max_size": 10485760,
    "max_files": 5,
    "app_name": "MyApplication"
}
```

The log directory is created automatically if it does not exist.

### Configuration Parameters

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| log_directory | string | Yes | Directory where log files will be stored |
| filename | string | Yes | Name of the log file |
| max_size | integer ≥ 1 | Yes | Size in bytes at which the current log file is rotated |
| max_files | integer ≥ 1 | Yes | Total number of log files to keep, including the current one (`max_files - 1` backups; `1` keeps no backup) |

A missing or out-of-range `max_size` or `max_files` makes `initialize()` throw `std::runtime_error` naming the field.
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

Include the header (compile with `-I Logger/include` and link the sources under `Logger/src/`):

```cpp
#include "core/logger.h"

int main() {
    try {
        // Initialize the logger
        Logger::getInstance().initialize("config.json");
        LOG_INFO("DEVICE-001") << "Application started" << std::endl;

        // Log messages
        LOG_INFO("DEVICE-001") << "This is a info message" << std::endl;
        LOG_WARN("DEVICE-001") << "This is a warning message" << std::endl;
        LOG_ERROR("DEVICE-001") << "This is a error message" << std::endl;
        LOG_CRITICAL("DEVICE-001") << "This is a critical message" << std::endl;
        LOG_DEBUG("DEVICE-001") << "This is a debug message" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
```

### Output Format ( With device id )

#### Console Output (JSON)
Each entry is written as a single line (shown pretty-printed here for readability):
```json
{
  "@timestamp":"2024-11-08 18:00:27.884",
  "app.name":"MyApplication",
  "device.id":"DEVICE-001",
  "file":"main.cpp",
  "line":36,
  "log.level":"INFO",
  "log.logger":"main",
  "message.context":"Application started",
  "pod.id":"hare-pod-6c58ffdbb8-f2m6m"
}
```

#### File Output (Plain Text)
```
2024-11-08 18:00:27.884 [INFO] [main] Application started (file: main.cpp, line: 36) (app: MyApplication) (pod.id: hare-pod-6c58ffdbb8-f2m6m) (device: DEVICE-001)
```
### Output Format ( Without device id )

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
  "pod.id":"hare-pod-6c58ffdbb8-f2m6m"
}
```

#### File Output (Plain Text)
```
2024-11-08 18:00:27.884 [INFO] [main] Application started (file: main.cpp, line: 36) (app: MyApplication) (pod.id: hare-pod-6c58ffdbb8-f2m6m)
```
## Log Rotation

After each write, the logger checks the size of the current log file. Once it reaches `max_size`, the file is rotated:

1. Renames existing backup files by incrementing their number (the oldest backup is overwritten)
2. Moves the current log file to `0.<filename>` (with `max_files` set to 1, the current file is deleted instead)
3. Creates a new log file

Example of rotated files with `max_files` set to 5:
```
app.log      # Current log file
0.app.log    # Most recent backup
1.app.log    # Second most recent backup
2.app.log    # Third most recent backup
3.app.log    # Oldest backup
```

<!-- ## Directory Structure
```
Logger/
├── include/              # Header files
│   ├── logger.h/       # Level Struct
│   ├── config_file.h/  
│   └── podInfo_Retriever.h/  
├── src/              # Source files
│   ├── logger.cxx/       # Core functionality
│   ├── config_file.cxx/  # Additional modules
│   └── podInfo_Retriever.cxx/  # Get PODid
``` -->
## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

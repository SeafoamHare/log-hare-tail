include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
    URL_HASH SHA256=8ad598c73ad796e0d8280b082cebd82a630d73e73cd3c70057938a6501bba5d7
    DOWNLOAD_EXTRACT_TIMESTAMP ON
)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

# One executable per file, because Logger is a singleton with thread_local state:
# logger_uninitialized_test must never see initialize(), and logger_initialize_test
# needs a process where it has not been called yet.
# gtest_discover_tests then registers every test case with CTest, and CTest runs each
# case in its own process. Each test uses its pid in its temp paths, so they can run in parallel
set(LOGGER_TESTS
    config_file_test
    log_store_controller_test
    logger_initialize_test
    logger_stream_test
    logger_uninitialized_test
)

foreach(test_name IN LISTS LOGGER_TESTS)
    add_executable(${test_name} ${test_name}.cpp)
    target_link_libraries(${test_name} PRIVATE logger GTest::gtest_main)
    set_target_properties(${test_name} PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
    logger_target_defaults(${test_name})
    gtest_discover_tests(${test_name})
endforeach()

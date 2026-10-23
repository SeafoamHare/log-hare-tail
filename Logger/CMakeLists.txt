cmake_minimum_required(VERSION 3.25)
project(log_hare_tail LANGUAGES CXX)

option(LOGGER_BUILD_TESTS "Build the unit tests" ${PROJECT_IS_TOP_LEVEL})
option(LOGGER_WERROR "Treat compiler warnings as errors" OFF)
option(LOGGER_CLANG_TIDY "Run clang-tidy on the library and demo while compiling" OFF)

find_package(nlohmann_json 3 REQUIRED)
find_package(Threads REQUIRED)

if(LOGGER_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES clang-tidy REQUIRED)
endif()

# Warnings (and clang-tidy, when enabled) apply only to this project's own targets,
# never to googletest, which has its own warning settings
function(logger_target_defaults target)
    target_compile_options(${target} PRIVATE -Wall -Wextra $<$<BOOL:${LOGGER_WERROR}>:-Werror>)
    # CMake compiles absolute paths, so __FILE__ (the "file" field of every record) would expose
    # the build machine's directory layout. Map it back to a path relative to the project
    target_compile_options(${target} PRIVATE "-fmacro-prefix-map=${PROJECT_SOURCE_DIR}/=")
endfunction()

function(logger_enable_clang_tidy target)
    if(LOGGER_CLANG_TIDY)
        set_target_properties(${target} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
    endif()
endfunction()

add_library(logger STATIC
    src/core/LogLevel.cxx
    src/core/config_file.cxx
    src/core/log_store_controller.cxx
    src/core/logger.cxx
    src/core/podInfo_Retriever.cxx
    src/formatMessage/JsonMessageFormatter.cxx
    src/formatMessage/PlainMessageFormatter.cxx
)
target_include_directories(logger PUBLIC include)
target_link_libraries(logger PUBLIC nlohmann_json::nlohmann_json Threads::Threads)
# The README promises C++11, so the library and the demo are compiled as C++11 to keep that true.
# CXX_STANDARD is not inherited, so code that links the library can still use a newer standard
set_target_properties(logger PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
logger_target_defaults(logger)
logger_enable_clang_tidy(logger)

add_executable(logimp main.cpp)
target_link_libraries(logimp PRIVATE logger)
set_target_properties(logimp PROPERTIES CXX_STANDARD 11 CXX_STANDARD_REQUIRED ON CXX_EXTENSIONS OFF)
logger_target_defaults(logimp)
logger_enable_clang_tidy(logimp)

if(LOGGER_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

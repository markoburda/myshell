cmake_minimum_required(VERSION 3.10)
project(myshell CXX)

set(CMAKE_CXX_STANDARD 17)

# Options
option(WARNINGS_AS_ERRORS   "Treat compiler warnings as errors."    OFF)
option(ENABLE_SANITIZERS    "Use sanitizers to detect errors."      OFF)
option(ENABLE_CONAN         "Use Conan as a package manager."       OFF)

# Include fixed CMake configuration
include(cmake/config.cmake)

# Project source compilation
include_directories(inc)
add_library(operations SHARED src/operations.cpp inc/operations/operations.hpp)
add_executable(myshell src/main.cpp)
target_link_libraries(myshell operations)

# Add external packages
find_package(Boost 1.65.0 COMPONENTS program_options REQUIRED)
include_directories(${Boost_INCLUDE_DIR})
target_link_libraries(myshell Boost::program_options)

#Add external commands
include_directories(external/mycat/inc)
add_executable(mycat external/mycat/src/main.cpp)
add_library(mycat_operations SHARED external/mycat/src/operations.cpp external/mycat/inc/operations/operations.hpp)
target_link_libraries(mycat mycat_operations)
target_link_libraries(mycat Boost::program_options)
# Sanitizers configuration
if (ENABLE_SANITIZERS)
    set(SANITIZE_UNDEFINED ON)
    # Only one of Memory, Address, or Thread sanitizers is applicable at the time
    set(SANITIZE_MEMORY ON)
#    set(SANITIZE_ADDRESS ON)
#    set(SANITIZE_THREAD ON)
    find_package(Sanitizers)

    # CMake targets to which apply sanitizers
    add_sanitizers(operations myshell)
endif ()

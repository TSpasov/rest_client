cmake_minimum_required(VERSION 3.20)
project(rest_client LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)

# Collect all source and header files
file(GLOB_RECURSE SOURCES
    src/*.cpp
    src/*.hpp
    include/*.hpp
)

add_executable(moveit_cli ${SOURCES})


target_include_directories(moveit_cli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Link curl
target_link_libraries(moveit_cli PRIVATE CURL::libcurl)


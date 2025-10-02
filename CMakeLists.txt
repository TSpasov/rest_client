cmake_minimum_required(VERSION 3.20)
project(rest_client LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)

file(GLOB SOURCES "src/*.cpp" "src/*.hpp")
add_executable(moveit_cli ${SOURCES})

target_link_libraries(moveit_cli PRIVATE CURL::libcurl)

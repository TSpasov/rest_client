cmake_minimum_required(VERSION 3.20)
project(rest_client LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)

file(GLOB_RECURSE LIB_SOURCES
    src/*.cpp
)


file(GLOB_RECURSE LIB_HEADERS
    include/*.hpp
)

add_library(restlib ${LIB_SOURCES} ${LIB_HEADERS})

target_include_directories(restlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(restlib PUBLIC CURL::libcurl)

# CLI target
add_executable(moveit_cli cli/main.cpp)
target_link_libraries(moveit_cli PRIVATE restlib)

# GUI target
if(WIN32)
    add_executable(moveit_gui WIN32 gui/WinMain.cpp gui/app.rc)
    target_link_libraries(moveit_gui PRIVATE restlib)
endif()

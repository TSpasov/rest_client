cmake_minimum_required(VERSION 3.20)
project(rest_client LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CURL REQUIRED)

# Business logic library
file(GLOB_RECURSE LIB_SOURCES
    src/*.cpp
    include/*.hpp
)

add_library(restlib ${LIB_SOURCES})

target_include_directories(restlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(restlib PUBLIC CURL::libcurl)

# CLI target
add_executable(moveit_cli cli/main.cpp)
target_include_directories(moveit_cli PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(moveit_cli PRIVATE restlib)

# GUI target
if(WIN32)
    add_executable(moveit_gui
        WIN32
        gui/WinMain.cpp
        gui/app.rc        # <— add the resource script here
    )
    target_include_directories(moveit_gui PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include gui)
    target_link_libraries(moveit_gui PRIVATE restlib)
endif()


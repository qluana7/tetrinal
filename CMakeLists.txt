cmake_minimum_required(VERSION 3.21)

project(tetrinal_project
    VERSION 1.0.0
    DESCRIPTION "Simulate no-gravity tetris zen. It support SRS and damage system, all spin, bags, etc."
    LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(APP_NAME "tetrinal")

add_compile_definitions(APP_VERSION="${PROJECT_VERSION}")

if(NOT DISABLE_ANSI)
add_compile_definitions(ANSI_ENABLED=1)
endif()

set(CMAKE_CXX_FLAGS_DEBUG_INIT "-g -DDEBUG")

add_compile_options(-Wall -std=c++20)

include_directories(./include)

file(GLOB_RECURSE SRCS "./src/**.cpp")
add_executable(${APP_NAME} ${SRCS})

find_package(Curses REQUIRED)
target_include_directories(${APP_NAME} PRIVATE ${CURSES_INCLUDE_DIRS})
target_link_libraries(${APP_NAME} PRIVATE ${CURSES_LIBRARIES})

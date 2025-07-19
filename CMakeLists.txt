cmake_minimum_required(VERSION 3.21)

project(tetrinal_project
    VERSION 0.3.1
    DESCRIPTION "Simulate no-gravity tetris zen. It support SRS and damage system, all spin, bags, etc."
    LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(APP_NAME "tetrinal")

add_compile_definitions(APP_VERSION="${PROJECT_VERSION}")

if(NOT DISABLE_ANSI)
add_compile_definitions(ANSI_ENABLED=1)
endif()

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG=1)
endif()

add_compile_options(-Wall -std=c++20)

file(GLOB_RECURSE SRCS "./src/**.cpp")
add_executable(${APP_NAME} ${SRCS})

find_package(Curses REQUIRED)
find_package(nlohmann_json 3.2.0 REQUIRED)

target_link_libraries(${APP_NAME} PRIVATE
    ${CURSES_LIBRARIES}
    nlohmann_json::nlohmann_json
)

target_include_directories(${APP_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CURSES_INCLUDE_DIRS}
)
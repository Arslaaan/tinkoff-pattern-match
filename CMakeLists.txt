cmake_minimum_required(VERSION 3.0.0)
project(tinkoff-pattern-match VERSION 1.0.0)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
message(${OpenCV_INCLUDE_DIRS})

add_executable(tinkoff-pattern-match main.cpp)

target_link_libraries(tinkoff-pattern-match ${OpenCV_LIBS})
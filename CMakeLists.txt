cmake_minimum_required(VERSION 3.0.0)
project(tinkoff-pattern-match VERSION 1.0.0)

SET(CMAKE_CXX_STANDARD 17)
# SET(CMAKE_CXX_FLAGS_DEBUG "-O3")

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
message(${OpenCV_INCLUDE_DIRS})

include_directories(include)
add_executable(
    tinkoff-pattern-match
    src/main.cpp
    src/stepProfit.cpp
    src/stepOrder.cpp
    src/actions.cpp
    src/screenReader.cpp
    src/figureImages.cpp
    src/matrixPoint.cpp
    src/gameModel.cpp
    src/stepOrderGenerator.cpp
)

enable_testing()
add_executable(
    tinkoff-pattern-match-test
    src/test.cpp
    src/stepProfit.cpp
    src/stepOrder.cpp
    src/actions.cpp
    src/screenReader.cpp
    src/figureImages.cpp
    src/matrixPoint.cpp
    src/gameModel.cpp
    src/stepOrderGenerator.cpp
)

target_link_libraries(
    tinkoff-pattern-match-test
    GTest::gtest_main
    GTest::gmock
)

target_link_libraries(tinkoff-pattern-match-test ${OpenCV_LIBS})
target_link_libraries(tinkoff-pattern-match ${OpenCV_LIBS})

include(GoogleTest)
gtest_discover_tests(tinkoff-pattern-match-test)
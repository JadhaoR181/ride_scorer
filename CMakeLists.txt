cmake_minimum_required(VERSION 3.16)
project(ride_scorer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_library(ride_core src/csv_reader.cpp src/kinematics.cpp src/smoother.cpp
                      src/events.cpp src/summary.cpp)
target_include_directories(ride_core PUBLIC include)

add_executable(ride_cli src/main.cpp)
target_link_libraries(ride_cli PRIVATE ride_core)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(ride_tests tests/kinematics_test.cpp tests/events_test.cpp)
target_link_libraries(ride_tests PRIVATE ride_core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(ride_tests)
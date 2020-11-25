cmake_minimum_required(VERSION 3.10)

project(UnitTesting)

add_executable(kinect_tests src/kinect_tests.cpp)

target_link_libraries(kinect_tests gtest gtest_main pthread)

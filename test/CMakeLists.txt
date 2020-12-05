cmake_minimum_required(VERSION 3.10)

project(UnitTesting)

add_executable(kinect_tests
               kinect_tests/kinect_tests.cpp
               kinect_tests/fakes/libfreenect_fake.cpp
               kinect_tests/mocks/libfreenect_mock.cpp
               ../src/kinect.cpp
               ../src/kinect_frame.cpp
               ../src/cyclic_task.cpp)

target_link_libraries(kinect_tests gtest gtest_main pthread gmock)

target_include_directories(kinect_tests PRIVATE "../inc")
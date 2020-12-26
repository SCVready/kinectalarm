cmake_minimum_required(VERSION 3.10)

project(UnitTesting)

######## Kinect class ########
add_executable(kinect_tests
               kinect_tests/kinect_tests.cpp
               kinect_tests/fakes/libfreenect_fake.cpp
               kinect_tests/mocks/libfreenect_mock.cpp
               ../src/kinect.cpp
               ../src/kinect_frame.cpp
               ../src/cyclic_task.cpp)
target_link_libraries(kinect_tests gtest gtest_main pthread gmock freeimage)
target_compile_definitions(kinect_tests PRIVATE __STDC_CONSTANT_MACROS)
target_compile_definitions(kinect_tests PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")
target_include_directories(kinect_tests PRIVATE "../inc")

######## KinectFrame class ########
add_executable(kinect_frame_tests
               kinect_frame_tests/kinect_frame_tests.cpp
               ../src/kinect_frame.cpp)
target_link_libraries(kinect_frame_tests gtest gtest_main pthread gmock freeimage)
target_compile_definitions(kinect_frame_tests PRIVATE __STDC_CONSTANT_MACROS)
target_compile_definitions(kinect_frame_tests PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")
target_include_directories(kinect_frame_tests PRIVATE "../inc")
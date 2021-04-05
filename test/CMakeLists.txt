cmake_minimum_required(VERSION 3.10)

project(UnitTesting)

set(CMAKE_CXX_STANDARD 17)

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

######## Liveview class ########
add_executable(liveview_tests
               liveview_tests/liveview_tests.cpp
               common/mocks/kinect_mock.cpp
               liveview_tests/mocks/liveview_observer_mock.cpp
               ../src/liveview.cpp
               ../src/cyclic_task.cpp
               ../src/kinect_frame.cpp)
target_link_libraries(liveview_tests gtest gtest_main pthread gmock freeimage)
target_compile_definitions(liveview_tests PRIVATE __STDC_CONSTANT_MACROS)
target_compile_definitions(liveview_tests PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")
target_include_directories(liveview_tests PRIVATE "../inc")

######## Detection class ########
add_executable(detection_tests
               detection_tests/detection_tests.cpp
               common/mocks/kinect_mock.cpp
               detection_tests/mocks/detection_observer_mock.cpp
               ../src/detection.cpp
               ../src/cyclic_task.cpp
               ../src/kinect_frame.cpp)
target_link_libraries(detection_tests gtest gtest_main pthread gmock freeimage)
target_compile_definitions(detection_tests PRIVATE __STDC_CONSTANT_MACROS)
target_compile_definitions(detection_tests PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")
target_include_directories(detection_tests PRIVATE "../inc")

######## Alarm class ########
add_executable(alarm_tests
               alarm_tests/fakes/common_fake.cpp
               alarm_tests/fakes/kinect_factory_fake.cpp
               alarm_tests/fakes/alarm_module_factory_fake.cpp
               alarm_tests/mocks/alarm_module_mock.cpp
               common/mocks/kinect_mock.cpp
               common/fakes/state_persistence_factory_fakes.cpp
               ../src/alarm.cpp
               alarm_tests/alarm_tests.cpp)
target_link_libraries(alarm_tests gtest gtest_main pthread gmock)
target_compile_definitions(alarm_tests PRIVATE __STDC_CONSTANT_MACROS)
target_compile_definitions(alarm_tests PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")
target_include_directories(alarm_tests PRIVATE "../inc")

######## MessageBroker class ########
add_executable(message_broker_tests
               ../src/message_broker.cpp
               message_broker_tests/mocks/message_broker_observer_mock.cpp
               message_broker_tests/message_broker_tests.cpp)
target_link_libraries(message_broker_tests gtest gtest_main pthread gmock hiredis event event_pthreads)
target_compile_definitions(message_broker_tests PRIVATE __STDC_CONSTANT_MACROS)
target_compile_definitions(message_broker_tests PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")
target_include_directories(message_broker_tests PRIVATE "../inc")

######## StatePersistence class ########
add_executable(state_persistence_tests
               ../src/state_persistence.cpp
               state_persistence_tests/state_persistence_tests.cpp)
target_link_libraries(state_persistence_tests gtest gtest_main pthread gmock sqlite3)
target_compile_definitions(state_persistence_tests PRIVATE __STDC_CONSTANT_MACROS)
target_compile_definitions(state_persistence_tests PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")
target_include_directories(state_persistence_tests PRIVATE "../inc")
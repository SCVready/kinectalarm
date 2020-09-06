cmake_minimum_required (VERSION 3.10)

######## Project settings ########
PROJECT(kinectalarm)

######## Project variables ########
file(GLOB_RECURSE sources
    "src/*.cpp"
)
get_filename_component(file_excluded ${CMAKE_CURRENT_SOURCE_DIR}/src/video_stream.cpp ABSOLUTE)
list(REMOVE_ITEM sources "${file_excluded}")
get_filename_component(file_excluded ${CMAKE_CURRENT_SOURCE_DIR}/src/video.cpp ABSOLUTE)
list(REMOVE_ITEM sources "${file_excluded}")

set(libs_ffmpeg
    "avcodec"
    "avformat"
    "avutil"
    "swscale"
    "swresample"
)

set(libs
    "freenect"
    "freeimage"
    "pthread"
    "m"
    "dl"
    "hiredis"
    "event"
    "ssl"
    "crypto"
    "sqlite3"
    ${libs_ffmpeg}
)

set(inc_dirs
    "inc"
)

######## Executable ########
add_executable(kinectalarm ${sources})

######## Include directories ########
target_include_directories(kinectalarm PRIVATE ${inc_dirs})

######## Link libraries ########
target_link_libraries(kinectalarm ${libs})

######## Compile definitions ########
target_compile_definitions(kinectalarm PRIVATE __STDC_CONSTANT_MACROS)
target_compile_definitions(kinectalarm PRIVATE "$<$<CONFIG:DEBUG>:DEBUG>")

######## Compile options ########
target_compile_options(kinectalarm PRIVATE -Wall -Werror)

######## Compile fetatures ########
target_compile_features(kinectalarm PRIVATE cxx_std_17)
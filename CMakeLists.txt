cmake_minimum_required (VERSION 2.8.1)

######## Project settings ########
PROJECT(kinectalarm)
SET(LICENSE "TBD")

######## Build and include settings ########

add_definitions(-D__STDC_CONSTANT_MACROS)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations ")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpoison-system-directories ")

include_directories(
	/usr/include/libxml2
)

link_directories(
	${LINK_DIRECTORIES}
)

file(GLOB SOURCES
	"src/*.cpp"
)

add_executable(
	kinectalarm

	${SOURCES}
)

TARGET_LINK_LIBRARIES(
	kinectalarm
	freenect
	freeimage
	pthread
	xml2
	avcodec
	avformat
	avutil
	swscale
	swresample
	m
	dl
)

######## Install targets ########
INSTALL(TARGETS kinectalarm
	RUNTIME DESTINATION bin
)

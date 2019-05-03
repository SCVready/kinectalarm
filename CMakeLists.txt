cmake_minimum_required (VERSION 2.8.1)

######## Project settings ########
PROJECT(kinectalarm)
SET(LICENSE "TBD")

######## Build and include settings ########

add_definitions(-D__STDC_CONSTANT_MACROS)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations ")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG_ALARM")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ")

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
	avcodec
	avformat
	avutil
	swscale
	swresample
	m
	dl
	hiredis
	event
	ssl
	crypto
	sqlite3
)

######## Install targets ########
INSTALL(TARGETS kinectalarm
	RUNTIME DESTINATION bin
)

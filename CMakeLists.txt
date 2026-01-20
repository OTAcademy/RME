cmake_minimum_required(VERSION 3.31)

project(rme)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

# Compiler-specific flags
if(MSVC)
	set(CMAKE_CXX_FLAGS                "/W3 /wd4996 /wd4800 /wd4100 /wd4706 /EHsc")
	set(CMAKE_CXX_FLAGS_DEBUG          "/Od /Zi /D__DEBUG__")
	set(CMAKE_CXX_FLAGS_MINSIZEREL     "/O1 /DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE        "/O2 /DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /Zi")
else()
	set(CMAKE_CXX_FLAGS                "-Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-deprecated-declarations -Wno-overloaded-virtual -Wno-strict-aliasing -Wno-sign-compare -Wno-unused-function -Wunused-result")
	set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g -D__DEBUG__")
	set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE        "-O3 -DNDEBUG")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g")
endif()

find_package(OpenGL REQUIRED)

if(APPLE)
    set(CMAKE_PREFIX_PATH /usr/local/opt/libarchive)
endif()
find_package(LibArchive REQUIRED)

if(WIN32)
    set(Boost_THREADAPI win32)
endif()
find_package(Boost 1.34.0 COMPONENTS thread system REQUIRED)

find_package(wxWidgets COMPONENTS html aui gl adv core net base REQUIRED)

find_package(GLUT REQUIRED)
find_package(ZLIB REQUIRED)

include(${wxWidgets_USE_FILE})
include(source/CMakeLists.txt)
if(WIN32)
	add_executable(rme WIN32 ${rme_H} ${rme_SRC})
else()
	add_executable(rme ${rme_H} ${rme_SRC})
endif()

set_target_properties(rme PROPERTIES CXX_STANDARD 20)
set_target_properties(rme PROPERTIES CXX_STANDARD_REQUIRED ON)

include_directories(${Boost_INCLUDE_DIRS} ${LibArchive_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIR})
target_link_libraries(rme ${wxWidgets_LIBRARIES} ${Boost_LIBRARIES} ${LibArchive_LIBRARIES} ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} ${ZLIB_LIBRARIES})
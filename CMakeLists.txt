cmake_minimum_required(VERSION 3.28)

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


# wxWidgets_USE_FILE is set by vcpkg/system wxWidgets, but empty with Conan
if(wxWidgets_USE_FILE)
	include(${wxWidgets_USE_FILE})
endif()

find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(nanovg CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

include(source/CMakeLists.txt)
# WIN32 is required for wxWidgets app entry point (WinMain)
if(WIN32)
	add_executable(rme WIN32 ${rme_H} ${rme_SRC})
else()
	add_executable(rme ${rme_H} ${rme_SRC})
endif()

set_target_properties(rme PROPERTIES CXX_STANDARD 20)
set_target_properties(rme PROPERTIES CXX_STANDARD_REQUIRED ON)
target_compile_definitions(rme PRIVATE WXWIN_COMPATIBILITY_3_3)


# Link against wxWidgets (works with both vcpkg and Conan)
if(TARGET wxWidgets::wxWidgets)
	# Conan provides a single wxWidgets::wxWidgets target
	target_link_libraries(rme PRIVATE wxWidgets::wxWidgets)
else()
	# vcpkg/system provides wxWidgets_LIBRARIES variable
	target_link_libraries(rme PRIVATE ${wxWidgets_LIBRARIES})
endif()

# Include directories (for legacy find modules)
target_include_directories(rme PRIVATE
	${Boost_INCLUDE_DIRS}
	${LibArchive_INCLUDE_DIRS}
	${OPENGL_INCLUDE_DIR}
	${GLUT_INCLUDE_DIRS}
	${ZLIB_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/source
)

# Link other libraries (works with both vcpkg targets and Conan targets)
if(TARGET Boost::boost)
	target_link_libraries(rme PRIVATE Boost::boost Boost::thread Boost::system)
else()
	target_link_libraries(rme PRIVATE ${Boost_LIBRARIES})
endif()

if(TARGET LibArchive::LibArchive)
	target_link_libraries(rme PRIVATE LibArchive::LibArchive)
else()
	target_link_libraries(rme PRIVATE ${LibArchive_LIBRARIES})
endif()

if(TARGET GLUT::GLUT)
	target_link_libraries(rme PRIVATE GLUT::GLUT)
else()
	target_link_libraries(rme PRIVATE ${GLUT_LIBRARIES})
endif()

if(TARGET ZLIB::ZLIB)
	target_link_libraries(rme PRIVATE ZLIB::ZLIB)
else()
	target_link_libraries(rme PRIVATE ${ZLIB_LIBRARIES})
endif()

target_link_libraries(rme PRIVATE ${OPENGL_LIBRARIES})
target_link_libraries(rme PRIVATE glad::glad)
target_link_libraries(rme PRIVATE glm::glm)
target_link_libraries(rme PRIVATE nanovg::nanovg)
target_link_libraries(rme PRIVATE spdlog::spdlog)
cmake_minimum_required(VERSION 3.28)

project(rme)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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


find_package(ZLIB REQUIRED)


# wxWidgets_USE_FILE is set by vcpkg/system wxWidgets, but empty with Conan
if(wxWidgets_USE_FILE)
	include(${wxWidgets_USE_FILE})
endif()

find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

# nanovg: try vcpkg/Conan first, fallback to ext/nanovg
find_package(nanovg CONFIG QUIET)
if(NOT nanovg_FOUND)
	message(STATUS "nanovg not found via package manager, using ext/nanovg...")
	
	# Build nanovg from ext/nanovg source
	add_library(nanovg_lib STATIC
		${CMAKE_CURRENT_SOURCE_DIR}/ext/nanovg/src/nanovg.c
	)
	target_include_directories(nanovg_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ext/nanovg/src)
	add_library(nanovg::nanovg ALIAS nanovg_lib)
endif()

include(source/CMakeLists.txt)
# WIN32 is required for wxWidgets app entry point (WinMain)
if(WIN32)
	add_executable(rme WIN32 ${rme_H} ${rme_SRC})
else()
	add_executable(rme ${rme_H} ${rme_SRC})
endif()

set_target_properties(rme PROPERTIES CXX_STANDARD 20)
set_target_properties(rme PROPERTIES CXX_STANDARD_REQUIRED ON)
target_compile_definitions(rme PRIVATE WXWIN_COMPATIBILITY_3_3 NANOVG_GL3)


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

# Test executable for autoborder preview - uses same sources as main target
add_executable(test_autoborder_preview ${rme_H} ${rme_SRC} ${CMAKE_CURRENT_SOURCE_DIR}/source/test_autoborder_preview.cpp)
target_compile_definitions(test_autoborder_preview PRIVATE WXWIN_COMPATIBILITY_3_3 NANOVG_GL3)

# Include directories
target_include_directories(test_autoborder_preview PRIVATE
	${Boost_INCLUDE_DIRS}
	${LibArchive_INCLUDE_DIRS}
	${OPENGL_INCLUDE_DIR}
	${ZLIB_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/source
)

# Link libraries (same as main rme target)
if(TARGET wxWidgets::wxWidgets)
	target_link_libraries(test_autoborder_preview PRIVATE wxWidgets::wxWidgets)
else()
	target_link_libraries(test_autoborder_preview PRIVATE ${wxWidgets_LIBRARIES})
endif()

if(TARGET Boost::boost)
	target_link_libraries(test_autoborder_preview PRIVATE Boost::boost Boost::thread Boost::system)
else()
	target_link_libraries(test_autoborder_preview PRIVATE ${Boost_LIBRARIES})
endif()

if(TARGET LibArchive::LibArchive)
	target_link_libraries(test_autoborder_preview PRIVATE LibArchive::LibArchive)
else()
	target_link_libraries(test_autoborder_preview PRIVATE ${LibArchive_LIBRARIES})
endif()

if(TARGET ZLIB::ZLIB)
	target_link_libraries(test_autoborder_preview PRIVATE ZLIB::ZLIB)
else()
	target_link_libraries(test_autoborder_preview PRIVATE ${ZLIB_LIBRARIES})
endif()

target_link_libraries(test_autoborder_preview PRIVATE ${OPENGL_LIBRARIES})
target_link_libraries(test_autoborder_preview PRIVATE glad::glad)
target_link_libraries(test_autoborder_preview PRIVATE glm::glm)
target_link_libraries(test_autoborder_preview PRIVATE nanovg::nanovg)
target_link_libraries(test_autoborder_preview PRIVATE spdlog::spdlog)

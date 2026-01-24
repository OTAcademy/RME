cmake_minimum_required(VERSION 3.12)

project(rme)

if(POLICY CMP0072)
    cmake_policy(SET CMP0072 NEW) # Prefer GLVND
endif()

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW) # Use BoostConfig
endif()

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS                "/W3 /EHsc /D_CRT_SECURE_NO_WARNINGS")
    set(CMAKE_CXX_FLAGS_DEBUG          "/Od /Zi /D__DEBUG__")
    set(CMAKE_CXX_FLAGS_MINSIZEREL     "/O1 /DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE        "/O2 /DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "/O2 /Zi")
else()
    set(CMAKE_CXX_FLAGS                "-Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -Wno-deprecated-declarations -Wno-overloaded-virtual -Wno-strict-aliasing -Wno-sign-compare -Wno-unused-function -Wunused-result")
    set(CMAKE_CXX_FLAGS_DEBUG          "-O0 -g -D__DEBUG__")
    set(CMAKE_CXX_FLAGS_MINSIZEREL     "-Os -DNDEBUG")
    set(CMAKE_CXX_FLAGS_RELEASE        "-O4 -DNDEBUG")
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

# Lua scripting support
if(MSVC)
    # Windows/vcpkg specific configuration
    find_package(lua CONFIG REQUIRED)
    find_package(sol2 CONFIG REQUIRED)
    find_package(cpr CONFIG REQUIRED)
    find_package(nlohmann_json CONFIG REQUIRED)

    # Validate Lua target from vcpkg
    if(TARGET lua::lua)
        set(LUA_LIB_TARGET lua::lua)
    elseif(TARGET lua)
        set(LUA_LIB_TARGET lua)
    else()
        message(FATAL_ERROR "Lua target not found in vcpkg config!")
    endif()

    # Explicitly add Lua includes to global path to ensure sol2 finds 'lua.hpp'
    get_target_property(VCPKG_LUA_INCLUDES ${LUA_LIB_TARGET} INTERFACE_INCLUDE_DIRECTORIES)
    if(VCPKG_LUA_INCLUDES)
        include_directories(${VCPKG_LUA_INCLUDES})
    endif()

    add_compile_definitions(SOL_USING_CXX_LUA_HPP=1)

    set(LIBS_TO_LINK ${LUA_LIB_TARGET} cpr::cpr nlohmann_json::nlohmann_json sol2::sol2)

else()
    # Linux / Standard CMake Module configuration
    find_package(Lua REQUIRED)

    include(FetchContent)

    # sol2
    find_package(sol2 CONFIG QUIET)
    if(NOT sol2_FOUND)
        message(STATUS "sol2 not found via find_package, using FetchContent...")
        FetchContent_Declare(
            sol2
            GIT_REPOSITORY https://github.com/ThePhD/sol2.git
            GIT_TAG        v3.3.0
        )
        FetchContent_MakeAvailable(sol2)
        if(NOT TARGET sol2::sol2)
            if(TARGET sol2)
                add_library(sol2::sol2 ALIAS sol2)
            else()
                set(SOL2_INCLUDE_DIRS ${sol2_SOURCE_DIR}/include)
                include_directories(${SOL2_INCLUDE_DIRS})
                add_library(sol2::sol2 INTERFACE IMPORTED)
                set_target_properties(sol2::sol2 PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SOL2_INCLUDE_DIRS}")
            endif()
        endif()
    endif()

    # cpr
    find_package(cpr CONFIG QUIET)
    if(NOT cpr_FOUND)
        message(STATUS "cpr not found via find_package, using FetchContent...")
        FetchContent_Declare(
            cpr
            GIT_REPOSITORY https://github.com/libcpr/cpr.git
            GIT_TAG        1.10.5
        )
        FetchContent_MakeAvailable(cpr)
    endif()

    # nlohmann_json
    find_package(nlohmann_json CONFIG QUIET)
    if(NOT nlohmann_json_FOUND)
        message(STATUS "nlohmann_json not found via find_package, using FetchContent...")
        FetchContent_Declare(
            json
            GIT_REPOSITORY https://github.com/nlohmann/json.git
            GIT_TAG        v3.11.3
        )
        FetchContent_MakeAvailable(json)
    endif()

    set(LIBS_TO_LINK ${LUA_LIBRARIES} cpr::cpr nlohmann_json::nlohmann_json sol2::sol2)
endif()

include(${wxWidgets_USE_FILE})
include(source/CMakeLists.txt)
add_executable(rme ${rme_H} ${rme_SRC})

set_target_properties(rme PROPERTIES CXX_STANDARD 17)
set_target_properties(rme PROPERTIES CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/source ${Boost_INCLUDE_DIRS} ${LibArchive_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR} ${GLUT_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIR} ${LUA_INCLUDE_DIR} ${LUA_INCLUDE_DIRS})
target_link_libraries(rme ${wxWidgets_LIBRARIES} ${Boost_LIBRARIES} ${LibArchive_LIBRARIES} ${OPENGL_LIBRARIES} ${GLUT_LIBRARIES} ${ZLIB_LIBRARIES} ${LIBS_TO_LINK})

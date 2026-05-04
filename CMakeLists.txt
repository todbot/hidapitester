cmake_minimum_required(VERSION 3.15)

project(hidapitester LANGUAGES C)

# Get version from most recent git tag, fall back to date stamp
execute_process(
    COMMAND git describe --tags --abbrev=0
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE HIDAPITESTER_VERSION
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(NOT HIDAPITESTER_VERSION MATCHES "^v")
    string(TIMESTAMP HIDAPITESTER_VERSION "v%Y%m%d")
endif()
message(STATUS "hidapitester version: ${HIDAPITESTER_VERSION}")

include(FetchContent)

# Build HIDAPI as static library
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# In case if we have a local copy of HIDAPI
set(HIDAPI_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../hidapi" CACHE STRING "Path to HIDAPI source")
if(EXISTS "${HIDAPI_SOURCE_DIR}")
    message(STATUS "Using local HIDAPI from: ${HIDAPI_SOURCE_DIR}")

    add_subdirectory("${HIDAPI_SOURCE_DIR}" hidapi EXCLUDE_FROM_ALL)
else()
    message(STATUS "Fetching HIDAPI from git...")
    FetchContent_Declare(
        hidapi
        GIT_REPOSITORY https://github.com/libusb/hidapi.git
        GIT_TAG master
    )

    FetchContent_MakeAvailable(hidapi)
endif()

add_executable(hidapitester hidapitester.c)

if(MSVC)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        getopt_win
        GIT_REPOSITORY https://github.com/ludvikjerabek/getopt-win.git
        GIT_TAG v2.42.0
    )
    FetchContent_MakeAvailable(getopt_win)

    target_link_libraries(hidapitester PRIVATE getopt::getopt_static)
    target_compile_definitions(hidapitester PRIVATE STATIC_GETOPT)
endif()

target_link_libraries(hidapitester PRIVATE hidapi::hidapi)

target_compile_definitions(hidapitester PRIVATE 
    HIDAPITESTER_VERSION="${HIDAPITESTER_VERSION}"
)

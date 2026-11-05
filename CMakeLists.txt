cmake_minimum_required(VERSION 3.15...3.31)

project(gbcodec
    VERSION 0.1.0
    DESCRIPTION "GB2312 <-> Unicode (UTF-8/UTF-16) conversion library"
    HOMEPAGE_URL "https://github.com/leiddev/gbcodec"
    LANGUAGES C)

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(GBCODEC_IS_TOP_LEVEL ON)
else()
    set(GBCODEC_IS_TOP_LEVEL OFF)
endif()

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(GBCODEC_BUILD_CXX "Build the C++ convenience wrapper" ${GBCODEC_IS_TOP_LEVEL})
option(GBCODEC_BUILD_EXAMPLES "Build the example programs" ${GBCODEC_IS_TOP_LEVEL})
option(GBCODEC_BUILD_TESTS "Build the unit tests" ${GBCODEC_IS_TOP_LEVEL})
option(GBCODEC_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

if(GBCODEC_BUILD_CXX)
    enable_language(CXX)
endif()

# Single-configuration generators need an explicit default build type.
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        Debug Release RelWithDebInfo MinSizeRel)
endif()

# ---------------------------------------------------------------------------
# Warning helper
# ---------------------------------------------------------------------------
function(gbcodec_enable_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive- /utf-8)
        if(GBCODEC_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    else()
        target_compile_options(${target} PRIVATE
            -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion)
        if(GBCODEC_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
endfunction()

# ---------------------------------------------------------------------------
# Library: C core
# ---------------------------------------------------------------------------
add_library(gbcodec
    src/unicode_utility.c
    src/utf16_to_gb2312_table.c)
add_library(gbcodec::gbcodec ALIAS gbcodec)

target_include_directories(gbcodec PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
    $<INSTALL_INTERFACE:include>)

set_target_properties(gbcodec PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})

gbcodec_enable_warnings(gbcodec)

# ---------------------------------------------------------------------------
# Library: C++ convenience wrapper
# ---------------------------------------------------------------------------
if(GBCODEC_BUILD_CXX)
    add_library(gbcodec_cpp src/unicode_utility_cpp.cpp)
    add_library(gbcodec::gbcodec_cpp ALIAS gbcodec_cpp)

    target_link_libraries(gbcodec_cpp PUBLIC gbcodec)

    set_target_properties(gbcodec_cpp PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR})

    gbcodec_enable_warnings(gbcodec_cpp)
endif()

# ---------------------------------------------------------------------------
# Tests and examples
# ---------------------------------------------------------------------------
if(GBCODEC_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(GBCODEC_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

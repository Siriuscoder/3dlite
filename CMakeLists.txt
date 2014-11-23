cmake_minimum_required(VERSION 2.8.10)
project(3dlite)

if(UNIX)
include(FindPkgConfig)
endif()


set(CMAKE_CONFIGURATION_TYPES Debug Release)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

message(STATUS "Build configuration (${PROJECT_NAME}): ${CMAKE_BUILD_TYPE}")

# Specify build paths
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX) 
    set(DEBUG_COMPILER_FLAGS "-g3 -O0 -fstack-protector-all")
    set(RELEASE_COMPILER_FLAGS "-g0 -O3")
    set(CMAKE_C_FLAGS "-std=c99 -Wall")
    set(CMAKE_CXX_FLAGS "-std=c++11 -Wall")
    set(CMAKE_C_FLAGS_DEBUG ${DEBUG_COMPILER_FLAGS})
    set(CMAKE_C_FLAGS_RELEASE ${RELEASE_COMPILER_FLAGS})
    set(CMAKE_CXX_FLAGS_DEBUG ${DEBUG_COMPILER_FLAGS})
    set(CMAKE_CXX_FLAGS_RELEASE ${RELEASE_COMPILER_FLAGS})
    add_definitions(-fPIC -pedantic)
elseif(MSVC)
    add_definitions(/fp:fast /W4 /D_CRT_SECURE_NO_WARNINGS)
endif()

add_definitions(-DPLATFORM_${CMAKE_SYSTEM_NAME})

if(UNIX)
pkg_search_module(SDL2 REQUIRED sdl2)
    if(NOT SDL2_FOUND)
        message(FATAL_ERROR "SDL2 Required")
    endif()
find_package(Threads)
else()
include(${CMAKE_SOURCE_DIR}/CMake/FindSDL2.cmake)
    if(NOT SDL2_FOUND)
        message(FATAL_ERROR "SDL2 Required")
    endif()
endif()

include_directories("${PROJECT_SOURCE_DIR}/3dlite/include")

add_subdirectory("${PROJECT_SOURCE_DIR}/3dlite/")
add_subdirectory("${PROJECT_SOURCE_DIR}/tests/")

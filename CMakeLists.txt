cmake_minimum_required(VERSION 2.8.10)
project(3dlite)

set(CMAKE_CONFIGURATION_TYPES Debug Release)
message(STATUS "Build configuration (${PROJECT_NAME}): ${CMAKE_BUILD_TYPE}")

# Specify build paths
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX) 
set(CMAKE_C_FLAGS "-ansi -Wall")
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall")
set(CMAKE_C_FLAGS_DEBUG "-g3 -O0")
set(CMAKE_C_FLAGS_RELEASE "-g0 -O2")
set(CMAKE_CXX_FLAGS_DEBUG "-g3 -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-g0 -O2")
elseif(MSVC)
add_definitions(/fp:fast /W4)
endif()

include_directories("${PROJECT_SOURCE_DIR}/3dlite/include")

add_subdirectory("${PROJECT_SOURCE_DIR}/3dlite/")

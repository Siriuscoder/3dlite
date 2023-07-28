
#	This file is part of lite3d (Light-weight 3d engine).
#	Copyright (C) 2014  Sirius (Korolev Nikita)
#
#	Lite3D is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#
#	Lite3D is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with Lite3D.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.12.0)
project(lite3d)

set(CMAKE_LITE3D_TOP_DIR ${PROJECT_SOURCE_DIR})
include(${CMAKE_LITE3D_TOP_DIR}/CMake/lite3dCommon.cmake)
set(CMAKE_MODULE_PATH ${CMAKE_LITE3D_TOP_DIR} "${PROJECT_SOURCE_DIR}/CMake/")

enable_testing()

# Configuration
set(CMAKE_CONFIGURATION_TYPES Debug Release)
# Platform configuration
set(PLATFORM_TYPE PLATFORM_${CMAKE_SYSTEM_NAME} CACHE INTERNAL "PLATFORM_TYPE")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Specify build paths
if(MSVC)
    # In MSVC configuration folders created automatically  
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/)	
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)
else()

    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})
endif()

message(STATUS "Build configuration (${PROJECT_NAME}): ${CMAKE_BUILD_TYPE} : ${PLATFORM_TYPE}")

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX) 
    set(DEBUG_COMPILER_FLAGS "-g3 -O0 -fstack-protector-all")
    set(RELEASE_COMPILER_FLAGS "-g0 -O3")
    set(CMAKE_C_FLAGS "-std=c99 -Wall -Wno-format-security")
    set(CMAKE_CXX_FLAGS "-std=c++17 -Wall -Wno-deprecated-declarations -Wno-format-security")
    set(CMAKE_C_FLAGS_DEBUG ${DEBUG_COMPILER_FLAGS})
    set(CMAKE_C_FLAGS_RELEASE ${RELEASE_COMPILER_FLAGS})
    set(CMAKE_CXX_FLAGS_DEBUG ${DEBUG_COMPILER_FLAGS})
    set(CMAKE_CXX_FLAGS_RELEASE ${RELEASE_COMPILER_FLAGS})
    add_definitions(-fPIC -Werror -pedantic-errors -Wpedantic)
    add_definitions(-D${PLATFORM_TYPE})
elseif(MSVC)
    set(RELEASE_COMPILER_FLAGS "/O2 /Ot /Ob2")
    set(CMAKE_C_FLAGS_RELEASE ${RELEASE_COMPILER_FLAGS})
    set(CMAKE_CXX_FLAGS_RELEASE ${RELEASE_COMPILER_FLAGS})

    add_definitions(/fp:fast /W3 /D_CRT_SECURE_NO_WARNINGS /D_SCL_SECURE_NO_WARNINGS /std:c++17)
    if(MSVC11)
        add_definitions(/DGTEST_HAS_TR1_TUPLE=0 /DGTEST_USE_OWN_TR1_TUPLE=1)
    endif()
    add_definitions(/D${PLATFORM_TYPE})
endif()

option(ENABLE_METRICS "Enable code performance counters" OFF)
if(ENABLE_METRICS)
    if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX) 
        add_definitions(-DLITE3D_WITH_METRICS)
    elseif(MSVC)
        add_definitions(/DLITE3D_WITH_METRICS)
    endif()
endif()

if(UNIX)
    include(FindPkgConfig)
    find_package(Threads)
endif()

find_package(SDL2 REQUIRED)

find_package(Assimp)
if(NOT ASSIMP_FOUND)
    message(WARNING "Assimp library not found.. Some functionaly will be droped")
else()
    if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX) 
        add_definitions(-DINCLUDE_ASSIMP)
    elseif(MSVC)
        add_definitions(/DINCLUDE_ASSIMP)
    endif()
endif()

if(GRAPHIC_BACKEND STREQUAL GLES2)
    find_package(GLES2 REQUIRED)
    if(OPENGLES2_FOUND)
        add_definitions(-DWITH_GLES2)
        set(OPENGL_LIBRARIES ${OPENGLES2_LIBRARIES})
        set(OPENGL_INCLUDE_DIR ${OPENGLES2_INCLUDE_DIR})
    endif()
elseif(GRAPHIC_BACKEND STREQUAL GLES3)
    find_package(GLES3 REQUIRED)
    if(OPENGLES3_FOUND)
        add_definitions(-DWITH_GLES3)
        set(OPENGL_LIBRARIES ${OPENGLES3_LIBRARIES})
        set(OPENGL_INCLUDE_DIR ${OPENGLES3_INCLUDE_DIR})
    endif()
else()
    find_package(OpenGL REQUIRED)
    set(GRAPHIC_BACKEND "GLEW")
endif()

message(STATUS "GPU frontend: ${GRAPHIC_BACKEND}")

find_package(DevIL)
if(NOT IL_FOUND)
    message(FATAL_ERROR "DevIL library not found..")
endif()

option(BUILD_ENGINE_ONLY "Do not build samples, tests and other staff" OFF)

include_directories("${PROJECT_SOURCE_DIR}/lite3d/include")
include_directories("${PROJECT_SOURCE_DIR}/lite3dpp/include")
include_directories("${PROJECT_SOURCE_DIR}/lite3dpp_font/include")

add_subdirectory("${PROJECT_SOURCE_DIR}/lite3d/")
add_subdirectory("${PROJECT_SOURCE_DIR}/lite3dpp/")
add_subdirectory("${PROJECT_SOURCE_DIR}/lite3dpp_font/")

if(NOT BUILD_ENGINE_ONLY)
add_subdirectory("${PROJECT_SOURCE_DIR}/tests/")
add_subdirectory("${PROJECT_SOURCE_DIR}/samples/")
add_subdirectory("${PROJECT_SOURCE_DIR}/tools/")
endif()

option(SHOW_STATS "Show cmake variables" OFF)
if(SHOW_STATS)
    print_build_info()
endif()


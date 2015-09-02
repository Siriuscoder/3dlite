
#	This file is part of lite3d (Light-weight 3d engine).
#	Copyright (C) 2014  Sirius (Korolev Nikita)
#
#	Foobar is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#
#	Foobar is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with Foobar.  If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 2.8.9)
project(lite3d)

if(UNIX)
include(FindPkgConfig)
endif()

# Configuration
set(CMAKE_CONFIGURATION_TYPES Debug Release)
# Platform configuration
set(PLATFORM_TYPE PLATFORM_${CMAKE_SYSTEM_NAME})

# Specify build paths
if(MSVC)
	# In MSVC configuration folders created automatically  
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/)
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/)	
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/)
else()
	if(NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE Debug)
	endif()

	message(STATUS "Build configuration (${PROJECT_NAME}): ${CMAKE_BUILD_TYPE} : ${PLATFORM_TYPE}")
	
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
	set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})
endif()

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
    set(RELEASE_COMPILER_FLAGS "/O2 /Ot /Ob2")
    set(CMAKE_C_FLAGS_RELEASE ${RELEASE_COMPILER_FLAGS})
    set(CMAKE_CXX_FLAGS_RELEASE ${RELEASE_COMPILER_FLAGS})

    add_definitions(/fp:fast /W3 /D_CRT_SECURE_NO_WARNINGS /D_SCL_SECURE_NO_WARNINGS)
	if(MSVC11)
		add_definitions(/DGTEST_HAS_TR1_TUPLE=0 /DGTEST_USE_OWN_TR1_TUPLE=1)
	endif()
endif()

add_definitions(-D${PLATFORM_TYPE})

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

find_package(OpenGL)
if(NOT OPENGL_FOUND)
    message(FATAL_ERROR "OpenGL library not found..")
endif()

include(FindDevIL)
if(NOT IL_FOUND)
    message(FATAL_ERROR "DevIL library not found..")
endif()

include(${CMAKE_SOURCE_DIR}/CMake/FindAssimp.cmake)
if(NOT ASSIMP_FOUND)
    message(FATAL_ERROR "Assimp library not found..")
endif()

include(${CMAKE_SOURCE_DIR}/CMake/lite3dCommon.cmake)

include_directories("${PROJECT_SOURCE_DIR}/lite3d/include")
include_directories("${PROJECT_SOURCE_DIR}/lite3dpp/include")

add_subdirectory("${PROJECT_SOURCE_DIR}/lite3d/")
add_subdirectory("${PROJECT_SOURCE_DIR}/lite3dpp/")
add_subdirectory("${PROJECT_SOURCE_DIR}/tests/")
add_subdirectory("${PROJECT_SOURCE_DIR}/samples/")
add_subdirectory("${PROJECT_SOURCE_DIR}/tools/")

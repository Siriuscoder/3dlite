
#	This file is part of 3dlite (Light-weight 3d engine).
#	Copyright (C) 2024 Sirius (Korolev Nikita)
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

# setup sources 
macro(source_files DIRECTORY)
    set(SOURCES_DIR	"${PROJECT_SOURCE_DIR}/${DIRECTORY}")
    message(STATUS "Source location: ${SOURCES_DIR}")
    
    file(GLOB SRC_C_FILES "${SOURCES_DIR}/*.c")
    file(GLOB SRC_CPP_FILES "${SOURCES_DIR}/*.cpp")
    
    list(APPEND SOURCES_LIST ${SRC_C_FILES})
    list(APPEND SOURCES_LIST ${SRC_CPP_FILES})
    
    get_filename_component(DIR_NAME ${DIRECTORY} NAME)
    
    if(${DIR_NAME} STREQUAL "src" OR ${DIRECTORY} STREQUAL ".")
        unset(DIR_NAME)
    endif()
    
    source_group("Source Files\\${DIR_NAME}" FILES ${SRC_C_FILES} ${SRC_CPP_FILES})
endmacro()

macro(headers_files DIRECTORY)
    set(SOURCES_DIR	"${PROJECT_SOURCE_DIR}/${DIRECTORY}")
    message(STATUS "Headers location: ${SOURCES_DIR}")
    
    file(GLOB H_C_FILES "${SOURCES_DIR}/*.h")
    file(GLOB H_CPP_FILES "${SOURCES_DIR}/*.hpp")
    
    list(APPEND HEADERS_LIST ${H_C_FILES})
    list(APPEND HEADERS_LIST ${H_CPP_FILES})
    
    get_filename_component(DIR_NAME ${DIRECTORY} NAME)
    
    if(${DIR_NAME} MATCHES "lite3d" OR ${DIRECTORY} STREQUAL ".")
        unset(DIR_NAME)
    endif()
    
    source_group("Header Files\\${DIR_NAME}" FILES ${H_C_FILES} ${H_CPP_FILES})
endmacro()

# Сборка сампла, первый параметр папка где лежит сампл (имя сампла будет такое же), далее идет список библиотек для линковки.
macro(define_sample SAMPLE_NAME)
    message(STATUS "Sample: ${SAMPLE_NAME}")
    
    project(${SAMPLE_NAME})
    unset(SOURCES_LIST)
    unset(HEADERS_LIST)
    source_files(${SAMPLE_NAME})
    headers_files(${SAMPLE_NAME})
    
    set(ARG_LIBS ${ARGN})
    
    if(MSVC)
        set(SOURCES_LIST ${SOURCES_LIST} ${CMAKE_LITE3D_TOP_DIR}/winres/ResourceSample.rc)
    endif()

    add_executable(${SAMPLE_NAME} ${SOURCES_LIST} ${HEADERS_LIST})

    target_include_directories(${SAMPLE_NAME} PRIVATE 
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>"
        "$<BUILD_INTERFACE:${SDL2_INCLUDE_DIR}>"
        "$<BUILD_INTERFACE:${SAMPLE_ADDON_INCLUDES}>")
            
    if(MSVC)
        target_compile_definitions(${SAMPLE_NAME} PRIVATE
            "$<BUILD_INTERFACE:_WINEXE>")
        target_include_directories(${SAMPLE_NAME} PRIVATE 
            "$<BUILD_INTERFACE:${CMAKE_LITE3D_TOP_DIR}/winres/>")
    endif()
    
    foreach(argLib IN LISTS ARG_LIBS)
        target_link_libraries(${SAMPLE_NAME} ${argLib})
    endforeach()

    target_link_libraries(${SAMPLE_NAME} 
        lite3d
        ${SAMPLE_ADDON_LIBRARIES})
        
endmacro()

macro(print_build_info)
# ------------------------- Begin Generic CMake Variable Logging ------------------

# /*	C++ comment style not allowed	*/

MESSAGE( STATUS "")
MESSAGE( STATUS "")
# if you are building in-source, this is the same as CMAKE_SOURCE_DIR, otherwise 
# this is the top level directory of your build tree 
MESSAGE( STATUS "CMAKE_BINARY_DIR:         " ${CMAKE_BINARY_DIR} )
MESSAGE( STATUS "CMAKE_LITE3D_TOP_DIR:     " ${CMAKE_LITE3D_TOP_DIR} )
# if you are building in-source, this is the same as CMAKE_CURRENT_SOURCE_DIR, otherwise this 
# is the directory where the compiled or generated files from the current CMakeLists.txt will go to 
MESSAGE( STATUS "CMAKE_CURRENT_BINARY_DIR: " ${CMAKE_CURRENT_BINARY_DIR} )
# this is the directory, from which cmake was started, i.e. the top level source directory 
MESSAGE( STATUS "CMAKE_SOURCE_DIR:         " ${CMAKE_SOURCE_DIR} )
# this is the directory where the currently processed CMakeLists.txt is located in 
MESSAGE( STATUS "CMAKE_CURRENT_SOURCE_DIR: " ${CMAKE_CURRENT_SOURCE_DIR} )
# contains the full path to the top level directory of your build tree 
MESSAGE( STATUS "PROJECT_BINARY_DIR: " ${PROJECT_BINARY_DIR} )
# contains the full path to the root of your project source directory,
# i.e. to the nearest directory where CMakeLists.txt contains the PROJECT() command 
MESSAGE( STATUS "PROJECT_SOURCE_DIR: " ${PROJECT_SOURCE_DIR} )
# set this variable to specify a common place where CMake should put all executable files
# (instead of CMAKE_CURRENT_BINARY_DIR)
MESSAGE( STATUS "EXECUTABLE_OUTPUT_PATH: " ${EXECUTABLE_OUTPUT_PATH} )
# set this variable to specify a common place where CMake should put all libraries 
# (instead of CMAKE_CURRENT_BINARY_DIR)
MESSAGE( STATUS "LIBRARY_OUTPUT_PATH:     " ${LIBRARY_OUTPUT_PATH} )
# tell CMake to search first in directories listed in CMAKE_MODULE_PATH
# when you use FIND_PACKAGE() or INCLUDE()
MESSAGE( STATUS "CMAKE_MODULE_PATH: " ${CMAKE_MODULE_PATH} )
# this is the complete path of the cmake which runs currently (e.g. /usr/local/bin/cmake) 
MESSAGE( STATUS "CMAKE_COMMAND: " ${CMAKE_COMMAND} )
# this is the CMake installation directory 
MESSAGE( STATUS "CMAKE_ROOT: " ${CMAKE_ROOT} )
# this is the filename including the complete path of the file where this variable is used. 
MESSAGE( STATUS "CMAKE_CURRENT_LIST_FILE: " ${CMAKE_CURRENT_LIST_FILE} )
# this is linenumber where the variable is used
MESSAGE( STATUS "CMAKE_CURRENT_LIST_LINE: " ${CMAKE_CURRENT_LIST_LINE} )
# this is used when searching for include files e.g. using the FIND_PATH() command.
MESSAGE( STATUS "CMAKE_INCLUDE_PATH: " ${CMAKE_INCLUDE_PATH} )
# this is used when searching for libraries e.g. using the FIND_LIBRARY() command.
MESSAGE( STATUS "CMAKE_LIBRARY_PATH: " ${CMAKE_LIBRARY_PATH} )
# the complete system name, e.g. "Linux-2.4.22", "FreeBSD-5.4-RELEASE" or "Windows 5.1" 
MESSAGE( STATUS "CMAKE_SYSTEM: " ${CMAKE_SYSTEM} )
# the short system name, e.g. "Linux", "FreeBSD" or "Windows"
MESSAGE( STATUS "CMAKE_SYSTEM_NAME: " ${CMAKE_SYSTEM_NAME} )
# only the version part of CMAKE_SYSTEM 
MESSAGE( STATUS "CMAKE_SYSTEM_VERSION: " ${CMAKE_SYSTEM_VERSION} )
# the processor name (e.g. "Intel(R) Pentium(R) M processor 2.00GHz") 
MESSAGE( STATUS "CMAKE_SYSTEM_PROCESSOR: " ${CMAKE_SYSTEM_PROCESSOR} )
# is TRUE on all UNIX-like OS's, including Apple OS X and CygWin
MESSAGE( STATUS "UNIX: " ${UNIX} )
# is TRUE on Windows, including CygWin 
MESSAGE( STATUS "WIN32: " ${WIN32} )
# is TRUE on Apple OS X
MESSAGE( STATUS "APPLE: " ${APPLE} )
# is TRUE when using the MinGW compiler in Windows
MESSAGE( STATUS "MINGW: " ${MINGW} )
# is TRUE on Windows when using the CygWin version of cmake
MESSAGE( STATUS "CYGWIN: " ${CYGWIN} )
# is TRUE on Windows when using a Borland compiler 
MESSAGE( STATUS "BORLAND: " ${BORLAND} )
# Microsoft compiler 
MESSAGE( STATUS "MSVC: " ${MSVC} )
MESSAGE( STATUS "MSVC_IDE: " ${MSVC_IDE} )
# set this to true if you don't want to rebuild the object files if the rules have changed, 
# but not the actual source files or headers (e.g. if you changed the some compiler switches) 
MESSAGE( STATUS "CMAKE_SKIP_RULE_DEPENDENCY: " ${CMAKE_SKIP_RULE_DEPENDENCY} )
# since CMake 2.1 the install rule depends on all, i.e. everything will be built before installing. 
# If you don't like this, set this one to true.
MESSAGE( STATUS "CMAKE_SKIP_INSTALL_ALL_DEPENDENCY: " ${CMAKE_SKIP_INSTALL_ALL_DEPENDENCY} )
# If set, runtime paths are not added when using shared libraries. Default it is set to OFF
MESSAGE( STATUS "CMAKE_SKIP_RPATH: " ${CMAKE_SKIP_RPATH} )
# set this to true if you are using makefiles and want to see the full compile and link 
# commands instead of only the shortened ones 
MESSAGE( STATUS "CMAKE_VERBOSE_MAKEFILE: " ${CMAKE_VERBOSE_MAKEFILE} )
# this will cause CMake to not put in the rules that re-run CMake. This might be useful if 
# you want to use the generated build files on another machine. 
MESSAGE( STATUS "CMAKE_SUPPRESS_REGENERATION: " ${CMAKE_SUPPRESS_REGENERATION} )
# A simple way to get switches to the compiler is to use ADD_DEFINITIONS(). 
# But there are also two variables exactly for this purpose: 

# the compiler flags for compiling C sources 
MESSAGE( STATUS "CMAKE_C_FLAGS: " ${CMAKE_C_FLAGS} )
# the compiler flags for compiling C++ sources 
MESSAGE( STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS} )
# Choose the type of build.  Example: SET(CMAKE_BUILD_TYPE Debug) 
MESSAGE( STATUS "CMAKE_BUILD_TYPE: " ${CMAKE_BUILD_TYPE} )
# if this is set to ON, then all libraries are built as shared libraries by default.
MESSAGE( STATUS "BUILD_SHARED_LIBS: " ${BUILD_SHARED_LIBS} )
# the compiler used for C files 
MESSAGE( STATUS "CMAKE_C_COMPILER: " ${CMAKE_C_COMPILER} )
MESSAGE( STATUS "CMAKE_C_COMPILER_ID: " ${CMAKE_C_COMPILER_ID} )
# the compiler used for C++ files 
MESSAGE( STATUS "CMAKE_CXX_COMPILER: " ${CMAKE_CXX_COMPILER} )
MESSAGE( STATUS "CMAKE_CXX_COMPILER_ID: " ${CMAKE_CXX_COMPILER_ID} )
# the tools for creating libraries 
MESSAGE( STATUS "CMAKE_AR: " ${CMAKE_AR} )
MESSAGE( STATUS "CMAKE_RANLIB: " ${CMAKE_RANLIB} )
# ------------------------- End of Generic CMake Variable Logging ------------------
endmacro()
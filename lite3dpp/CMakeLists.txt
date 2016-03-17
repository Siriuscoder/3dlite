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
project(as)
project(lite3dpp)

################################################################
unset(SOURCES_LIST)
unset(HEADERS_LIST)

source_files("src/as")
headers_files("include/lite3dpp/as")

if(MSVC AND CMAKE_CL_64)
    enable_language(ASM_MASM)
    if(CMAKE_ASM_MASM_COMPILER_WORKS)
        set(SOURCES_LIST ${SOURCES_LIST} src/as/as_callfunc_x64_msvc_asm.asm)
    else()
        message(FATAL ERROR "MSVC x86_64 target requires a working assembler")
    endif()
endif()

add_library(as ${SOURCES_LIST} ${HEADERS_LIST})

target_include_directories(as PRIVATE 
"$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/lite3dpp/as>")

################################################################
unset(SOURCES_LIST)
unset(HEADERS_LIST)

source_files("src")
source_files("src/as_helpers")
source_files("src/json")
headers_files("include/lite3dpp")
headers_files("include/lite3dpp/as_helpers")
headers_files("include/lite3dpp/json")

if(MSVC)
set(SOURCES_LIST ${SOURCES_LIST} ${CMAKE_SOURCE_DIR}/winres/Resource.rc)
endif()

add_library(lite3dpp SHARED ${SOURCES_LIST} ${HEADERS_LIST})

if(MSVC)
target_compile_definitions(lite3dpp PRIVATE
"$<BUILD_INTERFACE:WIN_3DLITEPP_DLL>")
target_include_directories(lite3dpp PRIVATE 
"$<BUILD_INTERFACE:${SDL2_INCLUDE_DIR}>"
"$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/winres/>")
else()
target_compile_options(lite3dpp PRIVATE 
"$<BUILD_INTERFACE:${SDL2_CFLAGS}>")
endif()

target_link_libraries(lite3dpp
lite3d
as
kazmath)

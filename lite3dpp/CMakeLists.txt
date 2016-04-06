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

cmake_minimum_required(VERSION 2.8.9)
project(lite3dpp)

################################################################
unset(SOURCES_LIST)
unset(HEADERS_LIST)

source_files("src/as")
headers_files("include/lite3dpp/as")

if(UNIX AND AS_ARM)
	list(APPEND SOURCES_LIST src/as/as_callfunc_arm_gcc.S
		src/as/as_callfunc_arm_vita.S
		src/as/as_callfunc_arm_xcode.S)
elseif(MSVC AND AS_ARM)
	list(APPEND src/as/as_callfunc_arm_msvc.asm)
endif()

################################################################
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
endif()

target_include_directories(lite3dpp PRIVATE 
"$<BUILD_INTERFACE:${SDL2_INCLUDE_DIR}>"
"$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/winres/>"
"$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/lite3dpp/as>")

target_link_libraries(lite3dpp
lite3d)

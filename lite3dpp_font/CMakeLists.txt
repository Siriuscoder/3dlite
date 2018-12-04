#	This file is part of lite3d (Light-weight 3d engine).
#	Copyright (C) 2016  Sirius (Korolev Nikita)
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
project(lite3dpp_font)

# FreeType
message(STATUS "Checking FreeType...")
find_package(Freetype REQUIRED)

################################################################
unset(SOURCES_LIST)
unset(HEADERS_LIST)

source_files("src")
headers_files("include/lite3dpp_font")
headers_files("include")
################################################################
if(MSVC)
set(SOURCES_LIST ${SOURCES_LIST} ${CMAKE_LITE3D_TOP_DIR}/winres/Resource.rc)
endif()

add_library(lite3dpp_font SHARED ${SOURCES_LIST} ${HEADERS_LIST})

if(MSVC)
target_compile_definitions(lite3dpp_font PRIVATE
"$<BUILD_INTERFACE:WIN_3DLITEPP_FONT_DLL>")
endif()

target_include_directories(lite3dpp_font PRIVATE
"$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>"
"$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/lite3dpp_font/>"
"$<BUILD_INTERFACE:${FREETYPE_INCLUDE_DIRS}>"
"$<BUILD_INTERFACE:${SDL2_INCLUDE_DIR}>"
"$<BUILD_INTERFACE:${CMAKE_LITE3D_TOP_DIR}/winres/>")

target_link_libraries(lite3dpp_font
${FREETYPE_LIBRARIES}
lite3dpp)

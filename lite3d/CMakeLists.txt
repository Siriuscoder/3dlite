
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
project(lite3d)

unset(SOURCES_LIST)
unset(HEADERS_LIST)

######################### 7zdec ########################################
source_files("src/7zdec")
headers_files("include/lite3d/7zdec")
######################### glewc ########################################
if(GRAPHIC_BACKEND STREQUAL GLEW)
	source_files("src/glew")
	headers_files("include/lite3d/GL")
endif()
######################### kazmath ######################################
source_files("src/kazmath")
headers_files("include/lite3d/kazmath")
######################### primary lite3d ###############################
source_files("src")
headers_files("include/lite3d")

if(MSVC)
set(SOURCES_LIST ${SOURCES_LIST} ${CMAKE_SOURCE_DIR}/winres/Resource.rc)
endif()

add_library(lite3d SHARED ${SOURCES_LIST} ${HEADERS_LIST})

target_include_directories(lite3d PRIVATE 
"$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/lite3d/7zdec>"
"$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/lite3d/>"
"$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/lite3d/kazmath/>")
target_compile_definitions(lite3d PRIVATE
"$<BUILD_INTERFACE:_7ZIP_PPMD_SUPPPORT>"
"$<BUILD_INTERFACE:GLEW_NO_GLU>"
"$<BUILD_INTERFACE:GLEW_STATIC>")

if(MSVC)
target_compile_definitions(lite3d PRIVATE
"$<BUILD_INTERFACE:WIN_3DLITE_DLL>")
endif()

target_include_directories(lite3d PRIVATE 
"$<BUILD_INTERFACE:${OPENGL_INCLUDE_DIR}>"
"$<BUILD_INTERFACE:${SDL2_INCLUDE_DIR}>"
"$<BUILD_INTERFACE:${IL_INCLUDE_DIR}>"
"$<BUILD_INTERFACE:${ASSIMP_INCLUDE_DIR}>"
"$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/winres/>")

target_link_libraries(lite3d
${OPENGL_LIBRARIES}
${IL_LIBRARIES}
${ILU_LIBRARIES}
${SDL2_LIBRARY}
${ASSIMP_LIBRARY}
${CMAKE_THREAD_LIBS_INIT})


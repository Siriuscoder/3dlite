
#	This file is part of 3dlite (Light-weight 3d engine).
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

cmake_minimum_required(VERSION 2.8.10)
project(7zdec)
project(glewc)
project(3dlite)
project(kazmath)
######################### 7zdec ################################
unset(SOURCES_LIST)
unset(HEADERS_LIST)

source_files("src/7zdec")
headers_files("include/3dlite/7zdec")

add_library(7zdec ${SOURCES_LIST} ${HEADERS_LIST})
target_include_directories(7zdec PRIVATE 
"$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/3dlite/7zdec>")
target_compile_definitions(7zdec PRIVATE
"$<BUILD_INTERFACE:_7ZIP_PPMD_SUPPPORT>")
######################### glewc ################################
unset(SOURCES_LIST)
unset(HEADERS_LIST)

source_files("src/glew")
headers_files("include/3dlite/GL")

add_library(glewc ${SOURCES_LIST} ${HEADERS_LIST})
target_include_directories(glewc PRIVATE 
"$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/3dlite/>")
target_compile_definitions(glewc PRIVATE
"$<BUILD_INTERFACE:GLEW_NO_GLU>"
"$<BUILD_INTERFACE:GLEW_STATIC>")
######################### kazmath ##############################
unset(SOURCES_LIST)
unset(HEADERS_LIST)

source_files("src/kazmath")
headers_files("include/3dlite/kazmath")

add_library(kazmath ${SOURCES_LIST} ${HEADERS_LIST})
target_include_directories(kazmath PRIVATE 
"$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/3dlite/kazmath/>")
######################### 3dlite ###############################
unset(SOURCES_LIST)
unset(HEADERS_LIST)

source_files("src")
headers_files("include/3dlite")

add_library(3dlite SHARED ${SOURCES_LIST} ${HEADERS_LIST})

if(MSVC)
target_compile_definitions(3dlite PRIVATE
"$<BUILD_INTERFACE:WIN_3DLITE_DLL>")
target_include_directories(3dlite PRIVATE 
"$<BUILD_INTERFACE:${SDL2_INCLUDE_DIR}>"
"$<BUILD_INTERFACE:${IL_INCLUDE_DIR}>"
"$<BUILD_INTERFACE:${ASSIMP_INCLUDE_DIR}>")
set(SDL2_LIBS ${SDL2_LIBRARY})
else()
target_compile_options(3dlite PRIVATE 
"$<BUILD_INTERFACE:${SDL2_CFLAGS}>")
set(SDL2_LIBS ${SDL2_LIBRARIES})
endif()

target_compile_definitions(3dlite PRIVATE
"$<BUILD_INTERFACE:GLEW_STATIC>")

target_link_libraries(3dlite
7zdec
glewc
kazmath
${OPENGL_LIBRARIES}
${IL_LIBRARIES}
${ILU_LIBRARIES}
${SDL2_LIBS}
${ASSIMP_LIBRARY}
${CMAKE_THREAD_LIBS_INIT})


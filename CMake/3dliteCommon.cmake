
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
	
	if(${DIR_NAME} MATCHES "3dlite" OR ${DIRECTORY} STREQUAL ".")
		unset(DIR_NAME)
	endif()
	
	source_group("Header Files\\${DIR_NAME}" FILES ${H_C_FILES} ${H_CPP_FILES})
endmacro()

macro(define_sample SAMPLE_NAME SAMPLE_DIRECTORY)
	message(STATUS "Sample: ${SAMPLE_NAME}")
	
	project(${SAMPLE_NAME})
	unset(SOURCES_LIST)
	unset(HEADERS_LIST)
	source_files(${SAMPLE_DIRECTORY})
	headers_files(${SAMPLE_DIRECTORY})

	add_executable(${SAMPLE_NAME} ${SOURCES_LIST} ${HEADERS_LIST})

	target_include_directories(${SAMPLE_NAME} PRIVATE 
			"$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/${SAMPLE_DIRECTORY}>")
	if(MSVC)
		target_compile_definitions(${SAMPLE_NAME} PRIVATE
			"$<BUILD_INTERFACE:_WINEXE>")
		target_include_directories(${SAMPLE_NAME} PRIVATE 
			"$<BUILD_INTERFACE:${SDL2_INCLUDE_DIR}>")
	else()
		target_compile_options(${SAMPLE_NAME} PRIVATE 
			"$<BUILD_INTERFACE:${SDL2_CFLAGS}>")
	endif()
	
	target_link_libraries(${SAMPLE_NAME} 
		3dlite
		3dlitepp)
		
endmacro()
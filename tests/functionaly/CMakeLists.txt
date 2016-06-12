cmake_minimum_required(VERSION 2.8.10)
project(lite3d_tests)

unset(SOURCES_LIST)
unset(HEADERS_LIST)
source_files(".")
headers_files(".")

add_executable(lite3d_tests ${SOURCES_LIST} ${HEADERS_LIST})

target_include_directories(lite3d_tests PRIVATE 
"$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/tests/gtest>"
"$<BUILD_INTERFACE:${SDL2_INCLUDE_DIR}>")

if(MSVC)
target_compile_definitions(lite3d_tests PRIVATE
"$<BUILD_INTERFACE:_WINEXE>")
else()
target_compile_options(lite3d_tests PRIVATE 
"$<BUILD_INTERFACE:${SDL2_CFLAGS}>")
endif()

target_link_libraries(lite3d_tests 
gtest
lite3d
lite3dpp)

if(NOT MSVC)
# how to know output executable file path in MSVC compilation case?
add_custom_command(TARGET lite3d_tests POST_BUILD 
	COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lite3d_tests
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/media/
	COMMENT "Self-testing...")
endif()

cmake_minimum_required(VERSION 2.8.10)
project(3dlite_tests)

unset(SOURCES_LIST)
unset(HEADERS_LIST)
source_files(".")
headers_files(".")

add_executable(3dlite_tests ${SOURCES_LIST} ${HEADERS_LIST})

target_include_directories(3dlite_tests PRIVATE 
"$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/tests/gtest>")

if(MSVC)
target_compile_definitions(3dlite_tests PRIVATE
"$<BUILD_INTERFACE:_WINEXE>")
target_include_directories(3dlite_tests PRIVATE 
"$<BUILD_INTERFACE:${SDL2_INCLUDE_DIR}>")
else()
target_compile_options(3dlite_tests PRIVATE 
"$<BUILD_INTERFACE:${SDL2_CFLAGS}>")
endif()

target_link_libraries(3dlite_tests 
gtest
3dlite
3dlitepp)

if(NOT MSVC)
# how to know output executable file path in MSVC compilation case?
add_custom_command(TARGET 3dlite_tests POST_BUILD 
	COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/3dlite_tests
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/media/
	COMMENT "Self-testing...")
endif()

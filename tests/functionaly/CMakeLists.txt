cmake_minimum_required(VERSION 2.8.10)
project(3dlite_tests)

set(LITE3D_TEST_SOURCES 3dlite_file_cache_test.cpp
3dlite_vbo_test.cpp
3dlite_common_test.cpp)

add_executable(3dlite_tests ${LITE3D_TEST_SOURCES})

target_include_directories(3dlite_tests PRIVATE 
"$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/tests/gtest>"
"$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/3dlite/include>"
"$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/3dlite++/include>")

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
3dlite)

if(NOT MSVC)
# how to know output executable file path in MSVC compilation case?
add_custom_command(TARGET 3dlite_tests POST_BUILD 
	COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/3dlite_tests
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/media/
	COMMENT "Self-testing...")
endif()

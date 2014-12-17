cmake_minimum_required(VERSION 2.8.10)
project(3dlite_texture_test)

set(TEXTURE_TEST_SOURCES 3dlite_texture_test.c)

add_executable(3dlite_texture_test ${TEXTURE_TEST_SOURCES})

target_include_directories(3dlite_texture_test PRIVATE 
"$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/tests/gtest>"
"$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/3dlite/include>"
"$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/3dlite++/include>")

if(MSVC)
target_compile_definitions(3dlite_texture_test PRIVATE
"$<BUILD_INTERFACE:_WINEXE>")
target_include_directories(3dlite_texture_test PRIVATE 
"$<BUILD_INTERFACE:${SDL2_INCLUDE_DIR}>")
else()
target_compile_options(3dlite_texture_test PRIVATE 
"$<BUILD_INTERFACE:${SDL2_CFLAGS}>")
endif()

target_link_libraries(3dlite_texture_test 
3dlite)


cmake_minimum_required(VERSION 2.8.10)
project(3dlite_sample_scene_node)
project(3dlite_sample_minigun)
project(3dlite_sample_rifle)

set(SAMPLE_SCENE_NODE_SOURCES 3dlite_sample_scene_node.c)
add_executable(3dlite_sample_scene_node ${SAMPLE_SCENE_NODE_SOURCES})

set(SAMPLE_MINIGUN_SOURCES 3dlite_sample_minigun.c)
add_executable(3dlite_sample_minigun ${SAMPLE_MINIGUN_SOURCES})

set(SAMPLE_RIFLE_SOURCES 3dlite_sample_rifle.c)
add_executable(3dlite_sample_rifle ${SAMPLE_RIFLE_SOURCES})

target_include_directories(3dlite_sample_scene_node PRIVATE 
"$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/3dlite/include>"
"$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/3dlite++/include>")
target_include_directories(3dlite_sample_minigun PRIVATE 
"$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/3dlite/include>"
"$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/3dlite++/include>")
target_include_directories(3dlite_sample_rifle PRIVATE 
"$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/3dlite/include>"
"$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/3dlite++/include>")

if(MSVC)
target_compile_definitions(3dlite_sample_scene_node PRIVATE
"$<BUILD_INTERFACE:_WINEXE>")
target_include_directories(3dlite_sample_scene_node PRIVATE 
"$<BUILD_INTERFACE:${SDL2_INCLUDE_DIR}>")
target_compile_definitions(3dlite_sample_minigun PRIVATE
"$<BUILD_INTERFACE:_WINEXE>")
target_include_directories(3dlite_sample_minigun PRIVATE 
"$<BUILD_INTERFACE:${SDL2_INCLUDE_DIR}>")
target_compile_definitions(3dlite_sample_rifle PRIVATE
"$<BUILD_INTERFACE:_WINEXE>")
target_include_directories(3dlite_sample_rifle PRIVATE 
"$<BUILD_INTERFACE:${SDL2_INCLUDE_DIR}>")
else()
target_compile_options(3dlite_sample_scene_node PRIVATE 
"$<BUILD_INTERFACE:${SDL2_CFLAGS}>")
target_compile_options(3dlite_sample_minigun PRIVATE 
"$<BUILD_INTERFACE:${SDL2_CFLAGS}>")
target_compile_options(3dlite_sample_rifle PRIVATE 
"$<BUILD_INTERFACE:${SDL2_CFLAGS}>")
endif()

target_link_libraries(3dlite_sample_scene_node 
3dlite)
target_link_libraries(3dlite_sample_minigun 
3dlite)
target_link_libraries(3dlite_sample_rifle 
3dlite)


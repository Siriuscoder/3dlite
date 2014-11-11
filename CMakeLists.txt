cmake_minimum_required(VERSION 2.8.10)
project(3dlite)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX) 
set(CMAKE_C_FLAGS "-ansi -Wall")
set(CMAKE_CXX_FLAGS "-std=c++11 -Wall")
elseif(MSVC)
add_definitions(/fp:fast /W4)
endif()

include_directories("${PROJECT_SOURCE_DIR}/3dlite/include")

add_subdirectory("${PROJECT_SOURCE_DIR}/3dlite/" 3dlite)

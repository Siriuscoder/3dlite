cmake_minimum_required(VERSION 2.8.10)
project(7zdec)
project(glewc)

set(7ZDEC_SOURCES src/7zdec/7zAlloc.c
src/7zdec/7zBuf2.c
src/7zdec/7zBuf.c
src/7zdec/7zCrc.c
src/7zdec/7zCrcOpt.c
src/7zdec/7zDec.c
src/7zdec/7zFile.c
src/7zdec/7zIn.c
src/7zdec/7zStream.c
src/7zdec/Bcj2.c
src/7zdec/Bra86.c
src/7zdec/Bra.c
src/7zdec/CpuArch.c
src/7zdec/Lzma2Dec.c
src/7zdec/LzmaDec.c
src/7zdec/Ppmd7.c
src/7zdec/Ppmd7Dec.c)

set(GLEWC_SOURCES src/glew/glew.c)

set(CMAKE_C_FLAGS "-Wall")


add_library(7zdec ${7ZDEC_SOURCES})
add_library(glewc ${GLEWC_SOURCES})

target_include_directories(7zdec PRIVATE 
"$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/7zdec>")
target_compile_definitions(7zdec PRIVATE
"$<BUILD_INTERFACE:_7ZIP_PPMD_SUPPPORT>")

target_include_directories(glewc PRIVATE 
"$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>")
target_compile_definitions(glewc PRIVATE
"$<BUILD_INTERFACE:GLEW_NO_GLU>"
"$<BUILD_INTERFACE:GLEW_STATIC>")





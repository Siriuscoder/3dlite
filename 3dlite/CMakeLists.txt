cmake_minimum_required(VERSION 2.8.10)
project(7zdec)
project(glewc)
project(3dlite)

################################################################
# 7zdec sources
set(7ZDEC_SOURCES src/7zdec/7zBuf2.c
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

set(7ZDEC_HEADERS include/3dlite/7zdec/7z.h
include/3dlite/7zdec/7zBuf.h
include/3dlite/7zdec/7zCrc.h
include/3dlite/7zdec/7zFile.h
include/3dlite/7zdec/7zVersion.h
include/3dlite/7zdec/Bcj2.h
include/3dlite/7zdec/Bra.h
include/3dlite/7zdec/CpuArch.h
include/3dlite/7zdec/Lzma2Dec.h
include/3dlite/7zdec/LzmaDec.h
include/3dlite/7zdec/Ppmd.h
include/3dlite/7zdec/Ppmd7.h
include/3dlite/7zdec/Types.h)

add_library(7zdec ${7ZDEC_SOURCES} ${7ZDEC_HEADERS})
target_include_directories(7zdec PRIVATE 
"$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/3dlite/7zdec>")
target_compile_definitions(7zdec PRIVATE
"$<BUILD_INTERFACE:_7ZIP_PPMD_SUPPPORT>")
################################################################
# glew sources
set(GLEWC_SOURCES src/glew/glew.c)

set(GLEWC_HEADERS include/3dlite/GL/glew.h
include/3dlite/GL/glxew.h
include/3dlite/GL/wglew.h)

add_library(glewc ${GLEWC_SOURCES} ${GLEWC_HEADERS})
target_include_directories(glewc PRIVATE 
"$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/3dlite/>")
target_compile_definitions(glewc PRIVATE
"$<BUILD_INTERFACE:GLEW_NO_GLU>"
"$<BUILD_INTERFACE:GLEW_STATIC>")
################################################################
# 3dlite sources
set(3DLITE_SOURCES src/nedmalloc.c
src/alloc.c
src/rb_tree.c
src/list.c
src/file_cache.c
src/7zloader.c)

set(3DLITE_HEADERS include/3dlite/alloc.h
include/3dlite/common.h
include/3dlite/nedmalloc.h
include/3dlite/rb_tree.h
include/3dlite/list.h
include/3dlite/file_cache.h
include/3dlite/7zloader.h)

add_library(3dlite ${3DLITE_SOURCES} ${3DLITE_HEADERS})

if(WIN32 AND MSVC)
target_compile_definitions(3dlite PRIVATE
"$<BUILD_INTERFACE:_WINDLL>")
target_include_directories(3dlite PRIVATE 
"$<BUILD_INTERFACE:${SDL2_INCLUDE_DIR}>")
else()
target_compile_options(3dlite PRIVATE 
"$<BUILD_INTERFACE:${SDL2_CFLAGS}>")
endif()

target_compile_definitions(3dlite PRIVATE
"$<BUILD_INTERFACE:PLATFORM_${CMAKE_SYSTEM_NAME}>")

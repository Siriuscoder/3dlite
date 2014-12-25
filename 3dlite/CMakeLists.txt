cmake_minimum_required(VERSION 2.8.10)
project(7zdec)
project(glewc)
project(3dlite)
project(kazmath)
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
# kazmath sources
set(KAZMATH_SOURCES src/kazmath/aabb.c
src/kazmath/mat3.c
src/kazmath/mat4.c
src/kazmath/plane.c
src/kazmath/quaternion.c
src/kazmath/ray2.c
src/kazmath/ray3.c
src/kazmath/utility.c
src/kazmath/vec2.c
src/kazmath/vec3.c
src/kazmath/vec4.c)

set(KAZMATH_HEADERS include/3dlite/kazmath/aabb.h
include/3dlite/kazmath/mat3.h
include/3dlite/kazmath/mat4.h
include/3dlite/kazmath/plane.h
include/3dlite/kazmath/quaternion.h
include/3dlite/kazmath/ray2.h
include/3dlite/kazmath/ray3.h
include/3dlite/kazmath/utility.h
include/3dlite/kazmath/vec2.h
include/3dlite/kazmath/vec3.h
include/3dlite/kazmath/vec4.h)

add_library(kazmath ${KAZMATH_SOURCES} ${KAZMATH_HEADERS})
target_include_directories(kazmath PRIVATE 
"$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/3dlite/kazmath/>")
################################################################
# 3dlite sources
set(3DLITE_SOURCES src/3dlite_nedmalloc.c
src/3dlite_alloc.c
src/3dlite_rb_tree.c
src/3dlite_list.c
src/3dlite_file_cache.c
src/3dlite_7zloader.c
src/3dlite_logger.c
src/3dlite_texture_unit.c
src/3dlite_video.c
src/3dlite_render.c
src/3dlite_main.c
src/3dlite_camera.c
src/3dlite_scene_node.c)

set(3DLITE_HEADERS include/3dlite/3dlite_alloc.h
include/3dlite/3dlite_common.h
include/3dlite/3dlite_nedmalloc.h
include/3dlite/3dlite_rb_tree.h
include/3dlite/3dlite_list.h
include/3dlite/3dlite_file_cache.h
include/3dlite/3dlite_7zloader.h
include/3dlite/3dlite_logger.h
include/3dlite/3dlite_texture_unit.h
include/3dlite/3dlite_video.h
include/3dlite/3dlite_render.h
include/3dlite/3dlite_main.h
include/3dlite/3dlite_camera.h
include/3dlite/3dlite_scene_node.h)

add_library(3dlite SHARED ${3DLITE_SOURCES} ${3DLITE_HEADERS})

if(MSVC)
target_compile_definitions(3dlite PRIVATE
"$<BUILD_INTERFACE:_WINDLL>")
target_include_directories(3dlite PRIVATE 
"$<BUILD_INTERFACE:${SDL2_INCLUDE_DIR}>"
"$<BUILD_INTERFACE:${IL_INCLUDE_DIR}>")
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
${CMAKE_THREAD_LIBS_INIT})


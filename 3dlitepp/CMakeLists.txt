cmake_minimum_required(VERSION 2.8.10)
project(as)
project(3dlitepp)

################################################################
# angelscript  sources
set(ANGELSCRIPT_HEADERS
    include/3dlitepp/as/angelscript.h
    include/3dlitepp/as/as_array.h
    include/3dlitepp/as/as_builder.h
    include/3dlitepp/as/as_bytecode.h
    include/3dlitepp/as/as_callfunc.h
    include/3dlitepp/as/as_compiler.h
    include/3dlitepp/as/as_config.h
    include/3dlitepp/as/as_configgroup.h
    include/3dlitepp/as/as_context.h
    include/3dlitepp/as/as_criticalsection.h
    include/3dlitepp/as/as_datatype.h
    include/3dlitepp/as/as_debug.h
    include/3dlitepp/as/as_generic.h
    include/3dlitepp/as/as_map.h
    include/3dlitepp/as/as_memory.h
    include/3dlitepp/as/as_module.h
    include/3dlitepp/as/as_objecttype.h
    include/3dlitepp/as/as_outputbuffer.h
    include/3dlitepp/as/as_parser.h
    include/3dlitepp/as/as_property.h
    include/3dlitepp/as/as_restore.h
    include/3dlitepp/as/as_scriptcode.h
    include/3dlitepp/as/as_scriptengine.h
    include/3dlitepp/as/as_scriptfunction.h
    include/3dlitepp/as/as_scriptnode.h
    include/3dlitepp/as/as_scriptobject.h
    include/3dlitepp/as/as_string.h
    include/3dlitepp/as/as_string_util.h
    include/3dlitepp/as/as_texts.h
    include/3dlitepp/as/as_thread.h
    include/3dlitepp/as/as_tokendef.h
    include/3dlitepp/as/as_tokenizer.h
    include/3dlitepp/as/as_typeinfo.h
    include/3dlitepp/as/as_variablescope.h)

set(ANGELSCRIPT_SOURCE
    src/as/as_atomic.cpp
    src/as/as_builder.cpp
    src/as/as_bytecode.cpp
    src/as/as_callfunc.cpp
    src/as/as_callfunc_x86.cpp
    src/as/as_callfunc_x64_gcc.cpp
    src/as/as_callfunc_x64_msvc.cpp
    src/as/as_callfunc_x64_mingw.cpp
    src/as/as_compiler.cpp
    src/as/as_configgroup.cpp
    src/as/as_context.cpp
    src/as/as_datatype.cpp
    src/as/as_gc.cpp
    src/as/as_generic.cpp
    src/as/as_globalproperty.cpp
    src/as/as_memory.cpp
    src/as/as_module.cpp
    src/as/as_objecttype.cpp
    src/as/as_outputbuffer.cpp
    src/as/as_parser.cpp
    src/as/as_restore.cpp
    src/as/as_scriptcode.cpp
    src/as/as_scriptengine.cpp
    src/as/as_scriptfunction.cpp
    src/as/as_scriptnode.cpp
    src/as/as_scriptobject.cpp
    src/as/as_string.cpp
    src/as/as_string_util.cpp
    src/as/as_thread.cpp
    src/as/as_tokenizer.cpp
    src/as/as_typeinfo.cpp
    src/as/as_variablescope.cpp)

if(MSVC AND CMAKE_CL_64)
    enable_language(ASM_MASM)
    if(CMAKE_ASM_MASM_COMPILER_WORKS)
        set(ANGELSCRIPT_SOURCE ${ANGELSCRIPT_SOURCE} src/as/as_callfunc_x64_msvc_asm.asm)
    else()
        message(FATAL ERROR "MSVC x86_64 target requires a working assembler")
    endif()
endif()

add_library(as ${ANGELSCRIPT_SOURCE} ${ANGELSCRIPT_HEADERS})

target_include_directories(as PRIVATE 
"$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/3dlitepp/as>")

################################################################
# 3dlitepp  sources

set(LITE3DPP_HEADERS
include/3dlitepp/as_helpers/debugger.h
include/3dlitepp/as_helpers/scriptarray.h
include/3dlitepp/as_helpers/scriptbuilder.h
include/3dlitepp/as_helpers/scriptdictionary.h
include/3dlitepp/as_helpers/scripthelper.h
include/3dlitepp/as_helpers/scriptmath.h
include/3dlitepp/as_helpers/scriptstdstring.h
include/3dlitepp/3dlitepp_as_runner.h
include/3dlitepp/3dlitepp_main.h
include/3dlitepp/3dlitepp_common.h)

set(LITE3DPP_SOURCE
src/as_helpers/debugger.cpp
src/as_helpers/scriptarray.cpp
src/as_helpers/scriptbuilder.cpp
src/as_helpers/scriptdictionary.cpp
src/as_helpers/scripthelper.cpp
src/as_helpers/scriptmath.cpp
src/as_helpers/scriptstdstring.cpp
src/as_helpers/scriptstdstring_utils.cpp
src/3dlitepp_as_runner.cpp
src/3dlitepp_main.cpp)

add_library(3dlitepp SHARED ${LITE3DPP_SOURCE} ${LITE3DPP_HEADERS})

if(MSVC)
target_compile_definitions(3dlitepp PRIVATE
"$<BUILD_INTERFACE:WIN_3DLITEPP_DLL>")
target_include_directories(3dlitepp PRIVATE 
"$<BUILD_INTERFACE:${SDL2_INCLUDE_DIR}>")
else()
target_compile_options(3dlitepp PRIVATE 
"$<BUILD_INTERFACE:${SDL2_CFLAGS}>")
endif()

target_link_libraries(3dlitepp
3dlite
as)

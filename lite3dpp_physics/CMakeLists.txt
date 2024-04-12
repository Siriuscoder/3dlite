#	This file is part of lite3d (Light-weight 3d engine).
#	Copyright (C) 2024  Sirius (Korolev Nikita)
#
#	Lite3D is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#
#	Lite3D is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with Lite3D.  If not, see <http://www.gnu.org/licenses/>.

project(lite3dpp_physics)

unset(SOURCES_LIST)
unset(HEADERS_LIST)

source_files("src")
headers_files("include/lite3dpp_physics")

if(MSVC)
    set(SOURCES_LIST ${SOURCES_LIST} ${CMAKE_LITE3D_TOP_DIR}/winres/Resource.rc)
endif()

add_library(lite3dpp_physics SHARED ${SOURCES_LIST} ${HEADERS_LIST})

if(MSVC)
    target_compile_definitions(lite3dpp_physics PRIVATE
        "$<BUILD_INTERFACE:WIN_3DLITEPP_PHYSICS_DLL>")
endif()

target_include_directories(lite3dpp_physics PRIVATE 
    "$<BUILD_INTERFACE:${SDL2_INCLUDE_DIR}>"
    "$<BUILD_INTERFACE:${BULLET_INCLUDE_DIRS}>"
    "$<BUILD_INTERFACE:${CMAKE_LITE3D_TOP_DIR}/winres/>")
    
target_link_libraries(lite3dpp_physics
    lite3d
    lite3dpp
    ${BULLET_LIBRARIES})

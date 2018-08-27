
#	This file is part of lite3d (Light-weight 3d engine).
#	Copyright (C) 2014  Sirius (Korolev Nikita)
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

cmake_minimum_required(VERSION 2.8.9)

define_sample(sample_scene_node sample_scene_node)
define_sample(sample_minigun sample_minigun)
define_sample(sample_rifle sample_rifle)
define_sample(sample_RTT sample_rtt)
define_sample(sample_objects sample_objects sample_common)
define_sample(sample_RTT2 sample_rtt2 sample_common)
define_sample(sample_dynamic_cube sample_dynamic_cube sample_common)
define_sample(sample_cellwallerkiller sample_cellwallerkiller sample_common)
define_sample(sample_sponzahall sample_sponzahall sample_common) 
define_sample(sample_vault sample_vault sample_common)
define_sample(sample_warship sample_warship sample_common)

find_package(Bullet)
if(NOT BULLET_FOUND)
    message(WARNING "Bullet library not found.. Physics simulation sample will be disabled")
else()
    set(SAMPLE_ADDON_INCLUDES ${BULLET_INCLUDE_DIRS})
    set(SAMPLE_ADDON_LIBS ${BULLET_LIBRARIES})
    define_sample(sample_physics sample_physics sample_common)
    unset(SAMPLE_ADDON_INCLUDES)
    unset(SAMPLE_ADDON_LIBS)
endif()

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
add_subdirectory(sample_common)

define_sample(sample_scene_node)
define_sample(sample_minigun)
define_sample(sample_rifle)
define_sample(sample_rtt)
define_sample(sample_objects sample_common lite3dpp_font lite3dpp)
define_sample(sample_rtt2 sample_common lite3dpp_font lite3dpp)
define_sample(sample_dynamic_cube sample_common lite3dpp_font lite3dpp)
define_sample(sample_cellwallerkiller sample_common lite3dpp_font lite3dpp)
define_sample(sample_sponzahall sample_common lite3dpp_font lite3dpp) 
define_sample(sample_vault sample_common lite3dpp_font lite3dpp)
define_sample(sample_warship sample_common lite3dpp_font lite3dpp)
define_sample(sample_viewer sample_common lite3dpp_font lite3dpp)
define_sample(sample_vault_111 sample_common lite3dpp_font lite3dpp)
define_sample(sample_reflection_sky sample_common lite3dpp_font lite3dpp)

find_package(Bullet)
if(NOT BULLET_FOUND)
    message(WARNING "Bullet library not found.. Physics simulation sample will be disabled")
else()
    set(SAMPLE_ADDON_INCLUDES ${BULLET_INCLUDE_DIRS})
    set(SAMPLE_ADDON_LIBRARIES ${BULLET_LIBRARIES})
    define_sample(sample_physics sample_common lite3dpp_font lite3dpp)
    unset(SAMPLE_ADDON_INCLUDES)
    unset(SAMPLE_ADDON_LIBRARIES)
endif()
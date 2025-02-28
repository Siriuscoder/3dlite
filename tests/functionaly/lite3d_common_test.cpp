/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2025 Sirius (Korolev Nikita)
 *
 *	Lite3D is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	Lite3D is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with Lite3D.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
#include <string.h>

#include "lite3d_common_test.h"

Lite3dCommon::Lite3dCommon()
{
    memset(&mSettings, 0, sizeof (mSettings));

    mSettings.logLevel = LITE3D_LOGLEVEL_ERROR;
    mSettings.textureSettings.anisotropy = 1;
    mSettings.textureSettings.useGLCompression = LITE3D_TRUE;
    mSettings.videoSettings.MSAA = 1;
    strcpy(mSettings.videoSettings.caption, "TEST window");
    mSettings.videoSettings.colorBits = 32;
    mSettings.videoSettings.fullscreen = LITE3D_FALSE;
    mSettings.videoSettings.screenWidth = 800;
    mSettings.videoSettings.screenHeight = 600;
    mSettings.videoSettings.vsync = LITE3D_TRUE;
    mSettings.videoSettings.hidden = LITE3D_TRUE;
}

bool Lite3dCommon::main()
{
    return lite3d_main(&mSettings) == LITE3D_TRUE;
}

/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2025  Sirius (Korolev Nikita)
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
#ifndef LITE3D_BUILTIN_SHADERS_H
#define LITE3D_BUILTIN_SHADERS_H

static const char *vs_builtin = 
#ifdef GLES
        "precision mediump float;\n"
#else
        "#version 330\n"
#endif
        "in vec4 vertexAttr; "
        "in vec3 normalAttr; "
        "in vec2 texCoordAttr; "
        "uniform mat4 projectionMatrix; "
        "uniform mat4 modelMatrix; "
        "uniform mat4 viewMatrix; "
        "out vec2 vTexCoord; "
        "void main() "
        "{"
        "   vTexCoord = texCoordAttr; "
        "   gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexAttr; "
        "}";

static const char *fs_builtin = 
#ifdef GLES
        "precision mediump float;\n"
#else
        "#version 330\n"
#endif
        "uniform sampler2D diffuse; "
        "in vec2 vTexCoord; "
        "out vec4 fragColor; "
        "void main() "
        "{"
        "   fragColor = texture(diffuse, vTexCoord.st); "
        "}";

#endif

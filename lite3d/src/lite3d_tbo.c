/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2015  Sirius (Korolev Nikita)
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
#include <SDL_log.h>
#include <SDL_assert.h>

#include <lite3d/lite3d_gl.h>
#include <lite3d/lite3d_glext.h>
#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_misc.h>
#include <lite3d/lite3d_tbo.h>

/*
Overview

    This extension provides a new texture type, called a buffer texture.
    Buffer textures are one-dimensional arrays of texels whose storage comes
    from an attached buffer object.  When a buffer object is bound to a buffer
    texture, a format is specified, and the data in the buffer object is
    treated as an array of texels of the specified format.

    The use of a buffer object to provide storage allows the texture data to
    be specified in a number of different ways:  via buffer object loads
    (BufferData), direct CPU writes (MapBuffer), framebuffer readbacks
    (EXT_pixel_buffer_object extension).  A buffer object can also be loaded
    by transform feedback (NV_transform_feedback extension), which captures
    selected transformed attributes of vertices processed by the GL.  Several
    of these mechanisms do not require an extra data copy, which would be
    required when using conventional TexImage-like entry points.

    Buffer textures do not support mipmapping, texture lookups with normalized
    floating-point texture coordinates, and texture filtering of any sort, and
    may not be used in fixed-function fragment processing.  They can be
    accessed via single texel fetch operations in programmable shaders.  For
    assembly shaders (NV_gpu_program4), the TXF instruction is used.  For GLSL
    (EXT_gpu_shader4), a new sampler type and texel fetch function are used.

    While buffer textures can be substantially larger than equivalent
    one-dimensional textures; the maximum texture size supported for buffer
    textures in the initial implementation of this extension is 2^27 texels,
    versus 2^13 (8192) texels for otherwise equivalent one-dimensional
    textures.  When a buffer object is attached to a buffer texture, a size is
    not specified; rather, the number of texels in the texture is taken by
    dividing the size of the buffer object by the size of each texel.
*/
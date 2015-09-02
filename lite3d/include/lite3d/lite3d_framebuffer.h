/******************************************************************************
*	This file is part of lite3d (Light-weight 3d engine).
*	Copyright (C) 2015  Sirius (Korolev Nikita)
*
*	Foobar is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	Foobar is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/
#ifndef LITE3D_FRAMEBUFFER_H
#define	LITE3D_FRAMEBUFFER_H

#include <lite3d/lite3d_common.h>
#include <lite3d/lite3d_list.h>
#include <lite3d/lite3d_texture_unit.h>

#define LITE3D_FRAMEBUFFER_STATUS_EMPTY     0x1
#define LITE3D_FRAMEBUFFER_STATUS_ERROR     0x2
#define LITE3D_FRAMEBUFFER_STATUS_OK        0x0 

typedef struct lite3d_framebuffer
{
    uint32_t framebufferId;
    uint32_t renderBuffersIds[3];
    size_t renderBuffersCount;
    uint8_t useColorbuffer;
    uint8_t useDepthbuffer;
    uint8_t useStencilbuffer;
    int32_t height;
    int32_t width;
    uint8_t status;
} lite3d_framebuffer;

LITE3D_CEXPORT int lite3d_framebuffer_technique_init(void);
LITE3D_CEXPORT int lite3d_framebuffer_init(lite3d_framebuffer *fb,
    int32_t width, int32_t height);

/* setup frame buffer attachments - without whis call framebuffer is invalid */
/* if attachments is NULL or attachments count is 0 renderbuffer attachment will be used */
LITE3D_CEXPORT int lite3d_framebuffer_setup(lite3d_framebuffer *fb,
    lite3d_texture_unit *colorAttachments, size_t colorAttachmentsCount, uint8_t useColorRenderbuffer,
    lite3d_texture_unit *depthAttachments, uint8_t useDepthRenderbuffer, uint8_t useStencilRenderbuffer);

LITE3D_CEXPORT int lite3d_framebuffer_screen_init(lite3d_framebuffer *fb, 
    int32_t width, int32_t height);

LITE3D_CEXPORT void lite3d_framebuffer_switch(lite3d_framebuffer *fb);
LITE3D_CEXPORT void lite3d_framebuffer_purge(lite3d_framebuffer *fb);

#endif	/* LITE3D_FRAMEBUFFER_H */


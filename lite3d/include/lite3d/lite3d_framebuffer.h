/******************************************************************************
*	This file is part of lite3d (Light-weight 3d engine).
*	Copyright (C) 2024 Sirius (Korolev Nikita)
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
#ifndef LITE3D_FRAMEBUFFER_H
#define	LITE3D_FRAMEBUFFER_H

#include <lite3d/lite3d_common.h>
#include <lite3d/lite3d_list.h>
#include <lite3d/lite3d_array.h>
#include <lite3d/lite3d_texture_unit.h>

#define LITE3D_FRAMEBUFFER_STATUS_OK                0x0
#define LITE3D_FRAMEBUFFER_STATUS_EMPTY             0x1
#define LITE3D_FRAMEBUFFER_STATUS_ERROR             0x2

#define LITE3D_FRAMEBUFFER_READ_RGB_INT8            0x1
#define LITE3D_FRAMEBUFFER_READ_RGBA_INT8           0x2
#define LITE3D_FRAMEBUFFER_READ_DEPTH_INT32         0x3
#define LITE3D_FRAMEBUFFER_READ_DEPTH_FLOAT32       0x4

#define LITE3D_FRAMEBUFFER_USE_COLOR_BUFFER         0x1
#define LITE3D_FRAMEBUFFER_USE_DEPTH_BUFFER         0x2
#define LITE3D_FRAMEBUFFER_USE_STENCIL_BUFFER       0x4
#define LITE3D_FRAMEBUFFER_USE_MSAA_X2              0x8
#define LITE3D_FRAMEBUFFER_USE_MSAA_X4              0x10
#define LITE3D_FRAMEBUFFER_USE_MSAA_X8              0x20
#define LITE3D_FRAMEBUFFER_USE_MSAA_X16             0x40
#define LITE3D_FRAMEBUFFER_USE_LAYERED_BINDING      0x80 // Support for layered framebuffer

typedef struct lite3d_framebuffer_layer
{
    uint8_t attachmentType;
    int32_t layer;
} lite3d_framebuffer_layer;

typedef struct lite3d_framebuffer_attachment
{
    lite3d_framebuffer_layer layer;
    lite3d_texture_unit *attachment;
} lite3d_framebuffer_attachment;

typedef struct lite3d_framebuffer_attachment_binding
{
    lite3d_framebuffer_attachment attachment;
    int32_t bindedLayer;
} lite3d_framebuffer_attachment_binding;

typedef struct lite3d_framebuffer
{
    uint32_t framebufferId;
    uint32_t renderBuffersIds[3];
    int8_t renderBuffersCount;
    uint32_t flags;
    int32_t height;
    int32_t width;
    uint8_t status;
    int32_t samples;
    int32_t rbIntFormat;
    lite3d_array colorAttachments;
    lite3d_framebuffer_attachment_binding depthAttachment;
} lite3d_framebuffer;

LITE3D_CEXPORT int lite3d_framebuffer_technique_init(void);
LITE3D_CEXPORT int lite3d_framebuffer_init(lite3d_framebuffer *fb,
    int32_t width, int32_t height);

/* setup frame buffer attachments - without whis call framebuffer is invalid */
/* if attachments is NULL or attachments count is 0 renderbuffer attachment will 
 * be used if corresponding flag is present 
 */
LITE3D_CEXPORT int lite3d_framebuffer_setup(lite3d_framebuffer *fb,
    const lite3d_framebuffer_attachment *attachments, size_t attachmentsCount, uint32_t flags);

/* Replace framebuffer attachments, framebuffer must be active */
LITE3D_CEXPORT int lite3d_framebuffer_replace(lite3d_framebuffer *fb,
    const lite3d_framebuffer_attachment *attachments, size_t attachmentsCount, uint32_t flags);

LITE3D_CEXPORT int lite3d_framebuffer_screen_init(lite3d_framebuffer *fb, 
    int32_t width, int32_t height);
LITE3D_CEXPORT void lite3d_framebuffer_resize(lite3d_framebuffer *fb, 
    int32_t width, int32_t height);

LITE3D_CEXPORT void lite3d_framebuffer_switch(lite3d_framebuffer *fb);
LITE3D_CEXPORT void lite3d_framebuffer_purge(lite3d_framebuffer *fb);

LITE3D_CEXPORT int lite3d_framebuffer_read(lite3d_framebuffer *fb,
    uint16_t index, uint16_t format, void *pixels);
LITE3D_CEXPORT size_t lite3d_framebuffer_size(lite3d_framebuffer *fb,
    uint8_t format);

LITE3D_CEXPORT int lite3d_framebuffer_blit(lite3d_framebuffer *from, lite3d_framebuffer *to);

LITE3D_CEXPORT void lite3d_framebuffer_switch_layer(lite3d_framebuffer *fb, const lite3d_framebuffer_layer *layer, 
    size_t layerCount);

LITE3D_CEXPORT void lite3d_framebuffer_rebuild_mipmaps(lite3d_framebuffer *fb);

#endif	/* LITE3D_FRAMEBUFFER_H */


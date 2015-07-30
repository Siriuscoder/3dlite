/******************************************************************************
 *	This file is part of 3dlite (Light-weight 3d engine).
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
#include <string.h>

#include <SDL_assert.h>
#include <SDL_log.h>

#include <3dlite/GL/glew.h>
#include <3dlite/3dlite_misc.h>
#include <3dlite/3dlite_main.h>
#include <3dlite/3dlite_framebuffer.h>

static lite3d_framebuffer *gCurrentFb = NULL;
static int gMaxColorAttachments = 0;
static int gMaxFramebufferSize = 0;

/*
Name

    _framebuffer_object

Name Strings

    GL_framebuffer_object

Overview

    This extension defines a simple interface for drawing to rendering
    destinations other than the buffers provided to the GL by the
    window-system.

    In this extension, these newly defined rendering destinations are
    known collectively as "framebuffer-attachable images".  This
    extension provides a mechanism for attaching framebuffer-attachable
    images to the GL framebuffer as one of the standard GL logical
    buffers: color, depth, and stencil.  (Attaching a
    framebuffer-attachable image to the accum logical buffer is left for
    a future extension to define).  When a framebuffer-attachable image
    is attached to the framebuffer, it is used as the source and
    destination of fragment operations as described in Chapter 4.

    By allowing the use of a framebuffer-attachable image as a rendering
    destination, this extension enables a form of "offscreen" rendering.
    Furthermore, "render to texture" is supported by allowing the images
    of a texture to be used as framebuffer-attachable images.  A
    particular image of a texture object is selected for use as a
    framebuffer-attachable image by specifying the mipmap level, cube
    map face (for a cube map texture), and z-offset (for a 3D texture)
    that identifies the image.  The "render to texture" semantics of
    this extension are similar to performing traditional rendering to
    the framebuffer, followed immediately by a call to CopyTexSubImage.
    However, by using this extension instead, an application can achieve
    the same effect, but with the advantage that the GL can usually
    eliminate the data copy that would have been incurred by calling
    CopyTexSubImage.

    This extension also defines a new GL object type, called a
    "renderbuffer", which encapsulates a single 2D pixel image.  The
    image of renderbuffer can be used as a framebuffer-attachable image
    for generalized offscreen rendering and it also provides a means to
    support rendering to GL logical buffer types which have no
    corresponding texture format (stencil, accum, etc).  A renderbuffer
    is similar to a texture in that both renderbuffers and textures can
    be independently allocated and shared among multiple contexts.  The
    framework defined by this extension is general enough that support
    for attaching images from GL objects other than textures and
    renderbuffers could be added by layered extensions.

    To facilitate efficient switching between collections of
    framebuffer-attachable images, this extension introduces another new
    GL object, called a framebuffer object.  A framebuffer object
    contains the state that defines the traditional GL framebuffer,
    including its set of images.  Prior to this extension, it was the
    window-system which defined and managed this collection of images,
    traditionally by grouping them into a "drawable".  The window-system
    API's would also provide a function (i.e., wglMakeCurrent,
    glXMakeCurrent, aglSetDrawable, etc.) to bind a drawable with a GL
    context (as is done in the WGL_ARB_pbuffer extension).  In this
    extension however, this functionality is subsumed by the GL and the
    GL provides the function BindFramebuffer to bind a framebuffer
    object to the current context.  Later, the context can bind back to
    the window-system-provided framebuffer in order to display rendered
    content.

    Previous extensions that enabled rendering to a texture have been
    much more complicated.  One example is the combination of
    ARB_pbuffer and ARB_render_texture, both of which are window-system
    extensions.  This combination requires calling MakeCurrent, an
    operation that may be expensive, to switch between the window and
    the pbuffer drawables.  An application must create one pbuffer per
    renderable texture in order to portably use ARB_render_texture.  An
    application must maintain at least one GL context per texture
    format, because each context can only operate on a single
    pixelformat or FBConfig.  All of these characteristics make
    ARB_render_texture both inefficient and cumbersome to use.

    _framebuffer_object, on the other hand, is both simpler to use
    and more efficient than ARB_render_texture.  The
    _framebuffer_object API is contained wholly within the GL API and
    has no (non-portable) window-system components.  Under
    _framebuffer_object, it is not necessary to create a second GL
    context when rendering to a texture image whose format differs from
    that of the window.  Finally, unlike the pbuffers of
    ARB_render_texture, a single framebuffer object can facilitate
    rendering to an unlimited number of texture objects.

Glossary of Helpful Terms

        logical buffer:
            One of the color, depth, or stencil buffers of the
            framebuffer.

        framebuffer:
            The collection of logical buffers and associated state
            defining where the output of GL rendering is directed.

        texture:
            an object which consists of one or more 2D arrays of pixel
            images and associated state that can be used as a source of
            data during the texture-mapping process described in section
            3.8.

        texture image:
            one of the 2D arrays of pixels that are part of a texture
            object as defined in section 3.8.  Texture images contain
            and define the texels of the texture object.

        renderbuffer:
            A new type of storage object which contains a single 2D
            array of pixels and associated state that can be used as a
            destination for pixel data written during the rendering
            process described in Chapter 4.

        renderbuffer image:
            The 2D array of pixels that is part of a renderbuffer
            object.  A renderbuffer image contains and defines the
            pixels of the renderbuffer object.

        framebuffer-attachable image:
            A 2D pixel image that can be attached to one of the logical
            buffer attachment points of a framebuffer object.  Texture
            images and renderbuffer images are two examples of
            framebuffer-attachable images.

        attachment point:
            The set of state which references a specific
            framebuffer-attachable image, and allows that
            framebuffer-attachable image to be used to store the
            contents of a logical buffer of a framebuffer object.  There
            is an attachment point state vector for each color, depth,
            and stencil buffer of a framebuffer.

        attach:
            The act of connecting one object to another object.

            An "attach" operation is similar to a "bind" operation in
            that both represent a reference to the attached or bound
            object for the purpose of managing object lifetimes and both
            enable manipulation of the state of the attached or bound
            object.

            However, an "attach" is also different from a "bind" in that
            "binding" an unused object creates a new object, while
            "attaching" does not.  Additionally, "bind" establishes a
            connection between a context and an object, while "attach"
            establishes a connection between two objects.

            Finally, if object "A" is attached to object "B" and object
            "B" is bound to context "C", then in most respects, we treat
            "A" as if it is <implicitly> bound to "C".

        framebuffer attachment completeness:
            Similar to texture "mipmap" or "cube" completeness from
            section 3.8.10, defines a minimum set of criteria for
            framebuffer attachment points.  (for complete definition,
            see section 4.4.4.1)

        framebuffer completeness:
            Similar to texture "mipmap cube completeness", defines a
            composite set of "completeness" requirements and
            relationships among the attached framebuffer-attachable
            images.  (for complete definition, see section 4.4.4.2)
 */

int lite3d_framebuffer_technique_init(void)
{
    if (!GLEW_VERSION_3_0)
    {
        if (!GLEW_EXT_framebuffer_object && !GLEW_ARB_framebuffer_object)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "%s: GLEW_framebuffer_object not supported..", __FUNCTION__);
            return LITE3D_FALSE;
        }
    }

    glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &gMaxColorAttachments);
    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &gMaxFramebufferSize);
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Framebuffer max color attachments: %d",
        gMaxColorAttachments);
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Framebuffer max size: %d",
        gMaxFramebufferSize);

    return LITE3D_TRUE;
}

int lite3d_framebuffer_init(lite3d_framebuffer *fb,
    int32_t width, int32_t height)
{
    SDL_assert(fb);

    if (glIsFramebuffer(fb->framebufferId))
        lite3d_framebuffer_purge(fb);

    lite3d_misc_gl_error_stack_clean();

    memset(fb, 0, sizeof (lite3d_framebuffer));
    fb->width = width;
    fb->height = height;

    glGenFramebuffers(1, &fb->framebufferId);
    fb->status = LITE3D_FRAMEBUFFER_STATUS_EMPTY;

    if (lite3d_misc_check_gl_error())
    {
        glDeleteFramebuffers(1, &fb->framebufferId);
        return LITE3D_FALSE;
    }

    return LITE3D_TRUE;
}

void lite3d_framebuffer_purge(lite3d_framebuffer *fb)
{
    SDL_assert(fb);

    if (!glIsFramebuffer(fb->framebufferId))
        return;

    glDeleteRenderbuffers(fb->renderBuffersCount, fb->renderBuffersIds);
    glDeleteFramebuffers(1, &fb->framebufferId);
}

int lite3d_framebuffer_setup(lite3d_framebuffer *fb,
    lite3d_texture_unit *colorAttachments, size_t colorAttachmentsCount, uint8_t useColorRenderbuffer,
    lite3d_texture_unit *depthAttachments, uint8_t useDepthRenderbuffer, uint8_t useStencilRenderbuffer)
{
    int renderBuffersCount = 0;
    SDL_assert(fb);
    SDL_assert_release(colorAttachmentsCount <= gMaxColorAttachments);

    if (fb->status != LITE3D_FRAMEBUFFER_STATUS_EMPTY)
        return LITE3D_FALSE;

    lite3d_misc_gl_error_stack_clean();
    glBindFramebuffer(GL_FRAMEBUFFER, fb->framebufferId);

    /* setup color attachment */
    if (colorAttachments && colorAttachmentsCount > 0)
    {
        size_t i = 0;
        for (; i < colorAttachmentsCount; i++)
        {
            switch (colorAttachments[i].textureTarget)
            {
                case LITE3D_TEXTURE_1D:
                {
                    glFramebufferTexture1D(GL_FRAMEBUFFER,
                        GL_COLOR_ATTACHMENT0 + i,
                        colorAttachments[i].textureTarget,
                        colorAttachments[i].textureID,
                        0);
                    colorAttachments[i].isFbAttachment = LITE3D_TRUE;
                    fb->useColorbuffer = LITE3D_TRUE;
                }
                    break;
                case LITE3D_TEXTURE_2D:
                {
                    glFramebufferTexture2D(GL_FRAMEBUFFER,
                        GL_COLOR_ATTACHMENT0 + i,
                        colorAttachments[i].textureTarget,
                        colorAttachments[i].textureID,
                        0);
                    colorAttachments[i].isFbAttachment = LITE3D_TRUE;
                    fb->useColorbuffer = LITE3D_TRUE;
                }
                    break;
                default:
                    /* other texture types not supported yet */
                    return LITE3D_FALSE;
            }
        }
    }
    else if (useColorRenderbuffer)
    {
        fb->useColorbuffer = LITE3D_TRUE;
        glGenRenderbuffers(1, &fb->renderBuffersIds[renderBuffersCount]);
        glBindRenderbuffer(GL_RENDERBUFFER,
            fb->renderBuffersIds[renderBuffersCount]);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, fb->width,
            fb->height);

        /* attach color buffer to FBO */
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_RENDERBUFFER, fb->renderBuffersIds[renderBuffersCount]);

        renderBuffersCount++;
    }

    if (lite3d_misc_check_gl_error())
    {
        lite3d_framebuffer_purge(fb);
        return LITE3D_FALSE;
    }

    /* setup color attachment */
    if (depthAttachments)
    {
        if (depthAttachments->textureTarget != LITE3D_TEXTURE_2D)
            return LITE3D_FALSE;

        fb->useDepthbuffer = LITE3D_TRUE;
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
            GL_TEXTURE_2D, depthAttachments->textureID, 0);
        depthAttachments->isFbAttachment = LITE3D_TRUE;
    }
    else if (useDepthRenderbuffer && useStencilRenderbuffer)
    {
        fb->useDepthbuffer = LITE3D_TRUE;
        fb->useStencilbuffer = LITE3D_TRUE;
        glGenRenderbuffers(1, &fb->renderBuffersIds[renderBuffersCount]);
        glBindRenderbuffer(GL_RENDERBUFFER,
            fb->renderBuffersIds[renderBuffersCount]);

        /* use dual depth/stencil buffer */
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
            fb->width, fb->height);
        /* Attach depth buffer to FBO */
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
            GL_RENDERBUFFER, fb->renderBuffersIds[renderBuffersCount]);
        /* Also attach as a stencil */
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
            GL_RENDERBUFFER, fb->renderBuffersIds[renderBuffersCount]);

        renderBuffersCount++;
    }
    else if (useDepthRenderbuffer)
    {
        fb->useDepthbuffer = LITE3D_TRUE;
        glGenRenderbuffers(1, &fb->renderBuffersIds[renderBuffersCount]);
        glBindRenderbuffer(GL_RENDERBUFFER,
            fb->renderBuffersIds[renderBuffersCount]);

        /* use dual depth/stencil buffer */
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
            fb->width, fb->height);
        /* Attach depth buffer to FBO */
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
            GL_RENDERBUFFER, fb->renderBuffersIds[renderBuffersCount]);

        renderBuffersCount++;
    }

    /* check errors */
    if (lite3d_misc_check_gl_error())
    {
        lite3d_framebuffer_purge(fb);
        return LITE3D_FALSE;
    }

    /*
     * This is similar to the case above (Color texture, Depth texture) 
     * except that since there is no color buffer, call glDrawBuffer(GL_NONE) 
     * before or after calling glBindFramebuffer(GL_FRAMEBUFFER, fb) 
     * and then render. When you are done, call glBindFramebuffer(GL_FRAMEBUFFER, 0)
     * to render to the main framebuffer. This is important, call glDrawBuffer(GL_BACK) 
     * after. If you call before glBindFramebuffer(GL_FRAMEBUFFER, 0), 
     * a GL error will be raised.
     */
    glDrawBuffer(fb->useColorbuffer ? GL_BACK : GL_NONE);
    glReadBuffer(fb->useColorbuffer ? GL_BACK : GL_NONE);

    /* Does the GPU support current FBO configuration? */
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        lite3d_framebuffer_purge(fb);
        return LITE3D_FALSE;
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "FBO: 0x%x validated, %dx%d, color %s, "
        "depth %s, stencil %s", fb->framebufferId, fb->width, fb->height,
        fb->useColorbuffer ? (colorAttachments ? "texture" : "renderbuffer") : "none",
        fb->useDepthbuffer ? (depthAttachments ? "texture" : "renderbuffer") : "none",
        fb->useStencilbuffer ? "renderbuffer" : "none");

    /* bind screen by current FBO */
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    fb->status = LITE3D_FRAMEBUFFER_STATUS_OK;
    return LITE3D_TRUE;
}

int lite3d_framebuffer_screen_init(lite3d_framebuffer *fb,
    int32_t width, int32_t height)
{
    SDL_assert(fb);
    memset(fb, 0, sizeof (lite3d_framebuffer));

    fb->framebufferId = 0; /* important: screen FBO`s id always = 0 ! */
    fb->width = width;
    fb->height = height;
    fb->status = LITE3D_FRAMEBUFFER_STATUS_OK;
    fb->useColorbuffer = fb->useDepthbuffer =
        fb->useStencilbuffer = LITE3D_TRUE;

    return LITE3D_TRUE;
}

void lite3d_framebuffer_switch(lite3d_framebuffer *fb)
{
    SDL_assert(fb);

    if (fb != gCurrentFb)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fb->framebufferId);
        gCurrentFb = fb;

        glDrawBuffer(gCurrentFb->useColorbuffer ? GL_BACK : GL_NONE);
        glReadBuffer(gCurrentFb->useColorbuffer ? GL_BACK : GL_NONE);
    }
}
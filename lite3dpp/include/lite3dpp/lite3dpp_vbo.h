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
#pragma once

#include <lite3dpp/lite3dpp_buffer_base.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT VBO : public BufferBase
    {
    public:

        using BufferBase::getData;
        using BufferBase::setData;

        VBO(lite3d_vbo &vbo);

        size_t bufferSizeBytes() const override;
        void extendBufferBytes(size_t addsize) override;
        void setBufferSizeBytes(size_t size) override;
        void setData(const void *buffer, size_t offset, size_t size) override;
        void getData(void *buffer, size_t offset, size_t size) const override;
        void replaceData(const void *buffer, size_t size) override;
        BufferScopedMapper map(BufferScopedMapper::BufferScopedMapperLockType lockType) override;
        bool valid() const override;

    private:

        lite3d_vbo &mVBO;
    };
    
    class LITE3DPP_EXPORT VBOResource : public ConfigurableResource, public VBO, public Noncopiable
    {
    public:

        using BufferBase::getData;
        using BufferBase::setData;
        
        VBOResource(const String &name, const String &path, Main *main,
            AbstractResource::ResourceType type);
        ~VBOResource();
        
        LITE3D_DECLARE_PTR_METHODS(lite3d_vbo, mVBO)
        
        size_t usedVideoMemBytes() const override;
        
    protected:
        
        virtual void loadFromConfigImpl(const ConfigurationReader &helper) override;
        virtual void unloadImpl() override;
        virtual void reloadFromConfigImpl(const ConfigurationReader &helper) override;
        
    private:

        lite3d_vbo mVBO;
        BufferData mVBOData;
    };
}

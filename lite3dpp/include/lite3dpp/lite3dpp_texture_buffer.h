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
#pragma once

#include <lite3d/lite3d_texture_unit.h>

#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/lite3dpp_config_reader.h>
#include <lite3dpp/lite3dpp_resource.h>
#include <lite3dpp/lite3dpp_texture.h>
#include <lite3dpp/lite3dpp_buffer_base.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT TextureBuffer : public Texture, public BufferBase
    {
    public:

        using BufferBase::getData;
        using BufferBase::setData;

        TextureBuffer(const String &name, 
            const String &path, Main *main);

        ~TextureBuffer();

        size_t bufferSizeTexels() const;
        uint8_t texelSize() const;
        
        size_t bufferSizeBytes() const override;
        void extendBufferBytes(size_t addsize) override;
        void setBufferSizeBytes(size_t size) override;
        void setData(const void *buffer, size_t offset, size_t size) override;
        void getData(void *buffer, size_t offset, size_t size) const override;
        BufferScopedMapper map(BufferScopedMapper::BufferScopedMapperLockType lockType) override;

        size_t usedVideoMemBytes() const override;

    protected:

        virtual void loadFromConfigImpl(const ConfigurationReader &helper) override;
        virtual void reloadFromConfigImpl(const ConfigurationReader &helper) override;
        virtual void unloadImpl() override;

    private:

        PixelsData mTexelsBackup;
    };
}


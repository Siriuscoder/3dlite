/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2016  Sirius (Korolev Nikita)
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
#include "src/lite3dpp_vault_deferred_shading.cpp"
#include "src/lite3dpp_vault_directrender.cpp"


int main(int agrc, char *args[])
{
    if (agrc > 1)
    {
        if (std::string(args[1]) == "DF")
        {
            lite3dpp::samples::VaultDF sample;
            return sample.start("vault_df/config/config_vault.json");
        }
        else if (std::string(args[1]) == "DR")
        {
            lite3dpp::samples::VaultDR sample;
            return sample.start("vault_dr/config/config_vault.json");
        }
        else if (std::string(args[1]) == "DRU")
        {
            lite3dpp::samples::VaultDR sample;
            return sample.start("vault_dr_ubo/config/config_vault.json");
        }
    }

    std::cout << "Usage: " << std::endl;
    std::cout << "\tsample_vault [MODE] " << std::endl << std::endl;
    std::cout << "\tMODE: " << std::endl;
    std::cout << "\tDF : Deferred Shading Render with lightmap pass" << std::endl;
    std::cout << "\tDR : Direct Render with TBO(Texture Buffer Object) lighting" << std::endl;
    std::cout << "\tDRU: Direct Render with UBO(Uniform Buffer Object) lighting" << std::endl;
    
    return -1;
}


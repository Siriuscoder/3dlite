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
        else if (std::string(args[1]) == "DR+SSBO")
        {
            lite3dpp::samples::VaultDR sample;
            return sample.start("vault_dr_ssbo/config/config_vault.json");
        }
    }

    lite3dpp::samples::VaultDF sample;
    return sample.start("vault_1/config/config_vault.json");
}


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
#pragma once

#include <lite3dpp/lite3dpp_main.h>

class Command : private lite3dpp::Main::LifecycleListener
{
public:

    Command();
    
    void run();
    void parseCommandLine(int argc, char *args[]);

protected:

    virtual void runImpl() = 0;
    virtual void parseCommandLineImpl(int argc, char *args[]);

    /* utils functions */
    lite3dpp::String makeFullPath(const lite3dpp::String &outputFolder, const lite3dpp::String &relative);
    lite3dpp::String makeRelativePath(const lite3dpp::String &inpath, 
        const lite3dpp::String &name, const lite3dpp::String &ext);
    void saveFile(const void *buffer, size_t size, const lite3dpp::String &path);
    void makeFolders(const lite3dpp::String &outputFolder);

private:

    virtual void init() override;
    virtual void shut() override;
    virtual void frameBegin() override;
    virtual void frameEnd() override;
    virtual void timerTick(lite3d_timer *timerid) override;
    virtual void processEvent(SDL_Event *e) override;

protected:

    lite3dpp::Main mMain;
    int mNonameCounter;
};
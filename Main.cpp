/*
===============================================================================
    Copyright (C) 2023-2025 Ilya Lyakhovets

    This program is free software : you can redistribute it and /or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.If not, see < http://www.gnu.org/licenses/>.
===============================================================================
*/

#include "Main.h"
#include "Tile.h"
#include "Game.h"

Game game;

/*
===================
Init
===================
*/
bool Init()
{
    LIB_CHECK(game.Init());
    engine->UnloadUnused();

    return true;
}

/*
===================
Draw
===================
*/
bool Draw()
{
    engine->ClearScreen(libColor(0.753f, 0.753f, 0.753f));
    game.Draw();

    return true;
}

/*
===================
Frame
===================
*/
bool Frame()
{
    if (engine->IsKeyPressed(LIBK_ESCAPE))
        engine->Stop();

    if (engine->IsKeyPressed(LIBK_F12))
        engine->TakeScreenshot();
    
    game.Update();

    return true;
}

/*
===================
Free
===================
*/
bool Free()
{
    game.SaveSettings();
    return true;
}

/*
===================
libMain
===================
*/
libMain()
{
    if (!libGetEngine(ENGINE_PATH))
    {
        libDialog::Error("Error!", "Couldn't load libEngine.");
        return -1;
    }

    engine->SetState(LIB_WINDOW_TITLE, "Minefield");
    engine->SetState(LIB_FPS_LIMIT, 30);
    engine->SetState(LIB_LOG_FILE, true);
    engine->SetState(LIB_LOG_FILENAME, "Minefield.log");
    engine->SetState(LIB_INIT, Init);
    engine->SetState(LIB_RENDER, Draw);
    engine->SetState(LIB_FRAME, Frame);
    engine->SetState(LIB_FREE, Free);

    engine->Start();
    libFreeEngine();

    return 0;
}

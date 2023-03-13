/*
===============================================================================
    Copyright (C) 2023 Ilya Lyakhovets

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

libCfg cfg;
Game game;

const char *help = "The rules are simple: click on a tile to reveal what's underneath. If it's a mine, you lose. If it's a number,\n"
                   "it tells you how many mines are adjacent to that tile. If it's blank, it means there are no mines nearby and\n"
                   "the surrounding tiles are automatically revealed. You can also right - click on a tile to mark it as a mine or as uncertain.\n"
                   "The goal is to clear all the tiles that don't have mines.\n\n"
                   "Copyright (C) 2023 Ilya Lyakhovets";

/*
===================
Init
===================
*/
bool Init()
{
    LIB_CHECK(game.Init());
    engine->UnloadUnused();

    engine->SetState(LIB_AUDIO_VOLUME, cfg.GetFloat("AudioVolume", DEFAULT_VOLUME));

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

    if (engine->IsKeyPressed(LIBK_F1))
        libDialog::Information(libFormat("Minefield version: %s", MINEFIELD_VERSION), help);

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
    cfg.SetFloat("AudioVolume", engine->GetState(LIB_AUDIO_VOLUME));
    cfg.SetFloat("MineRatio", game.mineRatio);
    cfg.SetInt("FieldWidth", game.fieldSize.x);
    cfg.SetInt("FieldHeight", game.fieldSize.y);
    cfg.Save();

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

    libStr path;
    libDir::GetLocalDataLocation(path);
    path.Append("/Minefield");
    libDir::Create(path.Get());
    path.Append("/Minefield.cfg");

    cfg.Load(path.Get(), DATA_PACK "Minefield.cfg");
    game.mineRatio = cfg.GetFloat("MineRatio", DEFAULT_MINE_RATIO);
    game.fieldSize.x = cfg.GetInt("FieldWidth", DEFAULT_FIELD_WIDTH);
    game.fieldSize.y = cfg.GetInt("FieldHeight", DEFAULT_FIELD_HEIGHT);

    game.ClampFieldDimensions();

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

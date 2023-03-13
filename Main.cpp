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

libTexture *t_curSmile = nullptr;
libTexture *t_smile = nullptr;
libTexture *t_smileClick = nullptr;
libTexture *t_smileWin = nullptr;
libTexture *t_smileLost = nullptr;
libTexture *t_tile = nullptr;
libTexture *t_tileOpen = nullptr;
libTexture *t_mine = nullptr;
libTexture *t_flag = nullptr;
libTexture *t_question = nullptr;
libSprite *t_boom = nullptr;
libSound *s_boom = nullptr;
libFont *font = nullptr;

libVec2 boom;
libQuad q_smile(libVertex(-TILE_SIZE, -TILE_SIZE, 0.0f, 0.0f), libVertex(TILE_SIZE, TILE_SIZE, 1.0f, 1.0f));
libQuad q_tile(libVertex(0.0f, 0.0f, 0.0f, 0.0f), libVertex(TILE_SIZE, TILE_SIZE, 1.0f, 1.0f));
libQuad q_boom(libVertex(-BOOM_RATIUS, -BOOM_RATIUS, 0.0f, 0.0f), libVertex(BOOM_RATIUS, BOOM_RATIUS, 1.0f, 1.0f));

enum gameState_t
{
    IN_GAME,
    WIN,
    LOST
} gameState;

struct Tile
{
    enum Type
    {
        EMPTY,
        MINE
    } type;

    enum State
    {
        CLOSED,
        OPEN,
        FLAGGED,
        QUESTIONED
    } state;

    int nearestMines;
    libButton button;
};

int gameTime = 0;
int attempts = 0;
int shownMinesLeft;
int minesLeft;
float mineRatio;
libVec2i fieldSize;
libVec2i newFieldSize;
libTimer timer;
libCfg cfg;
libButton buttonRestart(TILE_SIZE * 2, TILE_SIZE * 2);
Tile field[MAXIMAL_FIELD_WIDTH][MAXIMAL_FIELD_HEIGHT];

const char *help = "The rules are simple: click on a tile to reveal what's underneath. If it's a mine, you lose. If it's a number,\n"
                   "it tells you how many mines are adjacent to that tile. If it's blank, it means there are no mines nearby and\n"
                   "the surrounding tiles are automatically revealed. You can also right - click on a tile to mark it as a mine or as uncertain.\n"
                   "The goal is to clear all the tiles that don't have mines.\n\n"
                   "Copyright (C) 2023 Ilya Lyakhovets";

/*
===================
Restart
===================
*/
void Restart()
{
    fieldSize = newFieldSize;
    int numOfMines = libCast<int>(fieldSize.x * fieldSize.y * mineRatio);
    libArray<libVec2i> mines;

    for (int i = 0; i < fieldSize.x; i++)
    {
        for (int j = 0; j < fieldSize.y; j++)
        {
            field[i][j].type = Tile::EMPTY;
            field[i][j].nearestMines = 0;
            field[i][j].state = Tile::CLOSED;

            field[i][j].button.SetEnabled(true);
            field[i][j].button.SetTexture(t_tile);
            field[i][j].button.textureColor.base = libColor();
        }
    }

    // Gets possible mine locations
    for (int i = 0; i < fieldSize.x * fieldSize.y; i++)
        mines.Append(libVec2i(i % fieldSize.x, i / fieldSize.x));

    // Adds mines to the game field
    for (int i = 0; i < numOfMines; i++)
    {
        if (mines.IsEmpty())
            break;

        int n = libRandom::Int(0, mines.Size() - 1);
        field[mines[n].x][mines[n].y].type = Tile::MINE;
        mines.RemoveIndex(n);
    }

    // Calculates number of nearest mines for each tile
    for (int i = 0; i < fieldSize.x; i++)
    {
        for (int j = 0; j < fieldSize.y; j++)
        {
            if (field[i][j].type == Tile::MINE)
                continue;

            int mines = 0;

            for (int x = -1; x <= 1; x++)
            {
                for (int y = -1; y <= 1; y++)
                {
                    if (i == i + x && j == j + y)
                        continue;

                    if (i + x < 0 || i + x >= fieldSize.x || j + y < 0 || j + y >= fieldSize.y)
                        continue;

                    if (field[i + x][j + y].type == Tile::MINE)
                        mines++;
                }
            }

            field[i][j].nearestMines = mines;
        }
    }

    gameTime = 0;
    gameState = IN_GAME;
    shownMinesLeft = numOfMines;
    minesLeft = numOfMines;
    timer.Restart();
    t_boom->Reset();

    engine->SetState(LIB_WINDOW_SIZE, fieldSize.x * TILE_SIZE, libCast<int>(fieldSize.y * TILE_SIZE + buttonRestart.size.y));
}

/*
===================
ClampFieldDimensions
===================
*/
void ClampFieldDimensions()
{
    if (mineRatio < MINIMAL_MINE_RATIO)         mineRatio = MINIMAL_MINE_RATIO;
    if (mineRatio > MAXIMAL_MINE_RATIO)         mineRatio = MAXIMAL_MINE_RATIO;
    if (newFieldSize.x < MINIMAL_FIELD_WIDTH)   newFieldSize.x = MINIMAL_FIELD_WIDTH;
    if (newFieldSize.y < MINIMAL_FIELD_HEIGHT)  newFieldSize.y = MINIMAL_FIELD_HEIGHT;
    if (newFieldSize.x > MAXIMAL_FIELD_WIDTH)   newFieldSize.x = MAXIMAL_FIELD_WIDTH;
    if (newFieldSize.y > MAXIMAL_FIELD_HEIGHT)  newFieldSize.y = MAXIMAL_FIELD_HEIGHT;
}

/*
===================
Init
===================
*/
bool Init()
{
    LIB_CHECK(engine->GetTexture(t_smile, DATA_PACK "Textures/Smile.tga"));
    LIB_CHECK(engine->GetTexture(t_smileClick, DATA_PACK "Textures/SmileClick.tga"));
    LIB_CHECK(engine->GetTexture(t_smileWin, DATA_PACK "Textures/SmileWin.tga"));
    LIB_CHECK(engine->GetTexture(t_smileLost, DATA_PACK "Textures/SmileLost.tga"));
    LIB_CHECK(engine->GetTexture(t_tile, DATA_PACK "Textures/Tile.tga"));
    LIB_CHECK(engine->GetTexture(t_tileOpen, DATA_PACK "Textures/TileOpen.tga"));
    LIB_CHECK(engine->GetTexture(t_mine, DATA_PACK "Textures/Mine.tga"));
    LIB_CHECK(engine->GetTexture(t_flag, DATA_PACK "Textures/Flag.tga"));
    LIB_CHECK(engine->GetTexture(t_question, DATA_PACK "Textures/Question.tga"));
    LIB_CHECK(engine->GetSprite(t_boom, DATA_PACK "Textures/Boom/Boom.tga"));
    LIB_CHECK(engine->GetFont(font, DATA_PACK "Font.ttf"));
    LIB_CHECK(engine->GetSound(s_boom, DATA_PACK "Sounds/Boom.wav"));

    font->SetAlign(LIB_CENTER);
    font->SetSize(10);
    font->SetShadowShift(libVec2(1.0f, 1.0f));
    font->SetShadowType(libFont::SHADOW_ADDAPTIVE);

    s_boom->SetVolume(cfg.GetFloat("BoomVolume", DEFAULT_BOOM_VOLUME));
    t_boom->SetDuration(BOOM_DURATION);
    t_boom->SetStyle(libSprite::ONCE);

    t_curSmile = t_smile;

    engine->UnloadUnused();
    Restart();

    return true;
}

/*
===================
Draw
===================
*/
bool Draw()
{
    libVec2i screenSize(engine->GetState(LIB_SCREEN_WIDTH), engine->GetState(LIB_SCREEN_HEIGHT));
    gameTime = libCast<int>(timer.GetSeconds());

    engine->ClearScreen(libColor(0.753f, 0.753f, 0.753f));

    // Field
    for (int i = 0; i < fieldSize.x; i++)
    {
        for (int j = 0; j < fieldSize.y; j++)
        {
            float x = libCast<float>(TILE_SIZE * i);
            float y = libCast<float>(TILE_SIZE * (2 + j));

            // Tiles
            field[i][j].button.SetSize(TILE_SIZE, TILE_SIZE);
            field[i][j].button.SetPosition(libMath::Ceil(TILE_SIZE / 2.0f) + x, libMath::Ceil(TILE_SIZE / 2.0f + y));
            field[i][j].button.Draw();

            // Number of the nearest bombs around a tile
            if (field[i][j].type != Tile::MINE && field[i][j].state == Tile::OPEN)
            {
                if (field[i][j].nearestMines)
                {
                    if (field[i][j].nearestMines == 1)          font->SetColor(LIB_COLOR_AZURE);
                    else if (field[i][j].nearestMines == 2)     font->SetColor(LIB_COLOR_AO);
                    else if (field[i][j].nearestMines == 3)     font->SetColor(LIB_COLOR_RED);
                    else if (field[i][j].nearestMines == 4)     font->SetColor(LIB_COLOR_BLUE);
                    else if (field[i][j].nearestMines == 5)     font->SetColor(LIB_COLOR_MAROON);
                    else if (field[i][j].nearestMines == 6)     font->SetColor(LIB_COLOR_CYAN);
                    else if (field[i][j].nearestMines == 7)     font->SetColor(LIB_COLOR_BLACK);
                    else if (field[i][j].nearestMines == 8)     font->SetColor(LIB_COLOR_GRAY);

                    font->SetSize(libCast<int>(TILE_SIZE / 2));
                    font->Print2D(TILE_SIZE / 2.0f + x, TILE_SIZE / 2.0f + y, "%d", field[i][j].nearestMines);
                }
            }
            // Mines
            else if (gameState == LOST && ((field[i][j].type == Tile::MINE && field[i][j].state != Tile::FLAGGED) ||
                                           (field[i][j].type != Tile::MINE && field[i][j].state == Tile::FLAGGED)))
            {
                t_mine->Draw2DQuad(q_tile, x, y);

                // Cross out, which indicates wrongly placed flags
                if (gameState == LOST && field[i][j].type == Tile::EMPTY && field[i][j].state == Tile::FLAGGED)
                {
                    engine->Draw2DLine(libVertex(x, y, LIB_COLOR_RED),
                                       libVertex(x + TILE_SIZE, y + TILE_SIZE, LIB_COLOR_RED), 3.0f);

                    engine->Draw2DLine(libVertex(x, y + TILE_SIZE, LIB_COLOR_RED),
                                       libVertex(x + TILE_SIZE, y, LIB_COLOR_RED), 3.0f);
                }
            }
            // Flags
            if ((gameState != LOST && field[i][j].state == Tile::FLAGGED) ||
                (gameState == LOST && field[i][j].type == Tile::MINE && field[i][j].state == Tile::FLAGGED))
            {
                t_flag->Draw2DQuad(q_tile, x, y);
            }
            // Question marks
            else if (gameState == IN_GAME && field[i][j].state == Tile::QUESTIONED)
            {
                t_question->Draw2DQuad(q_tile, x, y);
            }
        }
    }

    // Mine explosion
    if (gameState == LOST && t_boom->IsPlaying())
        t_boom->Draw2DQuad(q_boom, boom.x, boom.y);

    buttonRestart.SetPosition(screenSize.x / 2.0f, buttonRestart.size.y / 2.0f);
    buttonRestart.Draw();
    t_curSmile->Draw2DQuad(q_smile, screenSize.x / 2.0f, TILE_SIZE);

    font->SetColor(LIB_COLOR_WHITE);
    font->SetSize(libCast<int>(TILE_SIZE * 0.68f));
    font->Print2D((screenSize.x - TILE_SIZE * 2) * 0.25f, TILE_SIZE, "Time: %d", gameTime > MAX_TIME ? MAX_TIME : gameTime);
    font->Print2D((screenSize.x + TILE_SIZE) * 0.75f, TILE_SIZE, "Mines: %d", shownMinesLeft);

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

    buttonRestart.Update();

    if (buttonRestart.IsPressed())
        buttonRestart.SetTexture(t_tileOpen);

    if (buttonRestart.IsReleased())
        Restart();

    // Sets smile button to default state
    if (!engine->IsKey(LIBK_MOUSE_LEFT))
    {
        if (gameState == IN_GAME)
            t_curSmile = t_smile;

        buttonRestart.SetTexture(t_tile);
    }

    // Tiles
    for (int i = 0, clicked = false; i < fieldSize.x; i++)
    {
        for (int j = 0; j < fieldSize.y; j++)
        {
            field[i][j].button.Update();

            if (clicked || gameState != IN_GAME)
                continue;

            if (field[i][j].button.IsPressed())
                if (field[i][j].state == Tile::CLOSED || field[i][j].state == Tile::QUESTIONED)
                    t_curSmile = t_smileClick;

            // Sets/unsets flags and question marks for tiles
            if (engine->IsKeyPressed(LIBK_MOUSE_RIGHT) && field[i][j].button.IsHovered() && field[i][j].state != Tile::OPEN)
            {
                // Flagged
                if(field[i][j].state == Tile::CLOSED)
                {
                    field[i][j].state = Tile::FLAGGED;
                    shownMinesLeft--;

                    if (field[i][j].type == Tile::MINE)
                        minesLeft--;
                }
                // Question mark
                else  if (field[i][j].state == Tile::FLAGGED)
                {
                    field[i][j].state = Tile::QUESTIONED;
                    shownMinesLeft++;
                }
                // Closed empty tile
                else  if (field[i][j].state == Tile::QUESTIONED)
                {
                    field[i][j].state = Tile::CLOSED;

                    if (field[i][j].type == Tile::MINE)
                        minesLeft++;
                }

                clicked = true;
            }

            // Opens a tile
            if (field[i][j].button.IsHovered() && engine->IsKeyReleased(LIBK_MOUSE_LEFT) &&
                (field[i][j].state == Tile::CLOSED || field[i][j].state == Tile::QUESTIONED))
            {
                field[i][j].button.SetTexture(t_tileOpen);
                field[i][j].state = Tile::OPEN;

                // Mine strike
                if (field[i][j].type == Tile::MINE)
                {
                    gameState = LOST;
                    t_curSmile = t_smileLost;
                    boom.Set(field[i][j].button.pos.x, field[i][j].button.pos.y);
                    t_boom->Play();
                    s_boom->Play();
                    field[i][j].button.textureColor.base = LIB_COLOR_RED;
                    field[i][j].button.SetEnabled(false);
                    timer.Stop();

                    // Shows all mines in closed tiles
                    for (int i = 0; i < fieldSize.x; i++)
                    {
                        for (int j = 0; j < fieldSize.y; j++)
                        {
                            if ((field[i][j].type == Tile::MINE && field[i][j].state != Tile::FLAGGED) ||
                                (field[i][j].type != Tile::MINE && field[i][j].state == Tile::FLAGGED))
                            {
                                field[i][j].button.SetTexture(t_tileOpen);
                            }
                        }
                    }

                    attempts++;

                    // Adjusts the level of difficulty
                    if (attempts >= ATTEMPTS_BEFORE_CHANGE)
                    {
                        attempts = 0;
                        mineRatio -= MINE_RATIO_CHANGE;

                        if (mineRatio < MINIMAL_MINE_RATIO)
                            mineRatio = MINIMAL_MINE_RATIO;
                    }

                    continue;
                }

                // Opens all nearest empty tiles
                if (field[i][j].type == Tile::EMPTY && !field[i][j].nearestMines)
                {
                    for (bool done = false; !done;)
                    {
                        done = true;

                        for (int i = 0; i < fieldSize.x; i++)
                        {
                            for (int j = 0; j < fieldSize.y; j++)
                            {
                                if (field[i][j].type == Tile::MINE || field[i][j].state == Tile::OPEN)
                                    continue;
                                
                                for (int x = -1; x <= 1; x++)
                                {
                                    for (int y = -1; y <= 1; y++)
                                    {
                                        if (i == i + x && j == j + y)
                                            continue;

                                        if (i + x < 0 || i + x >= fieldSize.x)
                                            continue;

                                        if (j + y < 0 || j + y >= fieldSize.y)
                                            continue;

                                        if (field[i + x][j + y].state == Tile::OPEN && !field[i + x][j + y].nearestMines)
                                        {
                                            if (field[i][j].state == Tile::FLAGGED)
                                                shownMinesLeft++;

                                            field[i][j].button.SetTexture(t_tileOpen);
                                            field[i][j].state = Tile::OPEN;
                                            done = false;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                bool isGameFinished = true;

                // Checks if there any closed empty tile left
                for (int i = 0; i < fieldSize.x; i++)
                {
                    for (int j = 0; j < fieldSize.y; j++)
                    {
                        if (field[i][j].type == Tile::EMPTY && field[i][j].state != Tile::OPEN)
                            isGameFinished = false;
                    }
                }

                if (isGameFinished)
                {
                    gameState = WIN;
                    t_curSmile = t_smileWin;
                    timer.Stop();

                    // Flags every closed mine tile
                    for (int i = 0; i < fieldSize.x; i++)
                    {
                        for (int j = 0; j < fieldSize.y; j++)
                        {
                            if (field[i][j].type == Tile::MINE && field[i][j].state != Tile::OPEN &&
                                                                  field[i][j].state != Tile::FLAGGED)
                            {
                                field[i][j].state = Tile::FLAGGED;
                                shownMinesLeft--;
                            }
                        }
                    }

                    // Adjusts the level of difficulty
                    attempts = 0;
                    mineRatio += MINE_RATIO_CHANGE;

                    if (gameTime < DESIRED_GAME_DURATION)
                    {
                        newFieldSize.x++;
                        newFieldSize.y++;
                    }
                    else
                    {
                        newFieldSize.x--;
                        newFieldSize.y--;
                    }

                    ClampFieldDimensions();
                }

                clicked = true;
            }
        }
    }

    return true;
}

/*
===================
Free
===================
*/
bool Free()
{
    cfg.SetFloat("BoomVolume", s_boom->GetVolume());
    cfg.SetFloat("MineRatio", mineRatio);
    cfg.SetInt("FieldWidth", fieldSize.x);
    cfg.SetInt("FieldHeight", fieldSize.y);
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
    mineRatio = cfg.GetFloat("MineRatio", DEFAULT_MINE_RATIO);
    newFieldSize.x = cfg.GetInt("FieldWidth", DEFAULT_FIELD_WIDTH);
    newFieldSize.y = cfg.GetInt("FieldHeight", DEFAULT_FIELD_HEIGHT);

    ClampFieldDimensions();

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

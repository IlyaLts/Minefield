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
#include "Game.h"

/*
===================
Game::Init
===================
*/
bool Game::Init()
{
    LIB_CHECK(engine->GetTexture(t_smile, DATA_PACK "Textures/Smile.tga"));
    LIB_CHECK(engine->GetTexture(t_smileClick, DATA_PACK "Textures/SmileClick.tga"));
    LIB_CHECK(engine->GetTexture(t_smileWin, DATA_PACK "Textures/SmileWon.tga"));
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

    t_boom->SetDuration(BOOM_DURATION);
    t_boom->SetStyle(libSprite::ONCE);

    t_curSmile = t_smile;

    engine->UnloadUnused();
    Restart();

    return true;
}

/*
===================
Game::Draw
===================
*/
void Game::Draw()
{
    libQuad q_smile(libVertex(-TILE_SIZE, -TILE_SIZE, 0.0f, 0.0f), libVertex(TILE_SIZE, TILE_SIZE, 1.0f, 1.0f));
    libQuad q_tile(libVertex(0.0f, 0.0f, 0.0f, 0.0f), libVertex(TILE_SIZE, TILE_SIZE, 1.0f, 1.0f));
    libQuad q_boom(libVertex(-BOOM_RATIUS, -BOOM_RATIUS, 0.0f, 0.0f), libVertex(BOOM_RATIUS, BOOM_RATIUS, 1.0f, 1.0f));

    libVec2i screenSize(engine->GetState(LIB_SCREEN_WIDTH), engine->GetState(LIB_SCREEN_HEIGHT));
    gameTime = libCast<int>(timer.GetSeconds());

    // Field
    for (int i = 0; i < fieldSize.x; i++)
    {
        for (int j = 0; j < fieldSize.y; j++)
        {
            float x = libCast<float>(TILE_SIZE * i);
            float y = libCast<float>(TILE_SIZE * (2 + j));
            Tile &tile = field[i][j];

            // Tiles
            tile.button.SetSize(TILE_SIZE, TILE_SIZE);
            tile.button.SetPosition(libMath::Ceil(TILE_SIZE / 2.0f) + x, libMath::Ceil(TILE_SIZE / 2.0f + y));
            tile.button.Draw();

            // Number of the nearest bombs around a tile
            if (tile.type != Tile::MINED && tile.state == Tile::OPEN)
            {
                if (tile.nearestMines)
                {
                    if (tile.nearestMines == 1)
                        font->SetColor(LIB_COLOR_AZURE);
                    else if (tile.nearestMines == 2)
                        font->SetColor(LIB_COLOR_AO);
                    else if (tile.nearestMines == 3)
                        font->SetColor(LIB_COLOR_RED);
                    else if (tile.nearestMines == 4)
                        font->SetColor(LIB_COLOR_BLUE);
                    else if (tile.nearestMines == 5)
                        font->SetColor(LIB_COLOR_MAROON);
                    else if (tile.nearestMines == 6)
                        font->SetColor(LIB_COLOR_CYAN);
                    else if (tile.nearestMines == 7)
                        font->SetColor(LIB_COLOR_BLACK);
                    else if (tile.nearestMines == 8)
                        font->SetColor(LIB_COLOR_GRAY);

                    font->SetSize(libCast<int>(TILE_SIZE / 2));
                    font->Print2D(TILE_SIZE / 2.0f + x, TILE_SIZE / 2.0f + y, "%d", tile.nearestMines);
                }
            }
            // Mines
            else if (gameState == LOST && tile.IsIncorrectlyFlaggedOrMined())
            {
                t_mine->Draw2DQuad(q_tile, x, y);

                // Cross out, which indicates wrongly placed flags
                if (gameState == LOST && tile.type == Tile::EMPTY && tile.state == Tile::FLAGGED)
                {
                    engine->Draw2DLine(libVertex(x, y, LIB_COLOR_RED),
                                       libVertex(x + TILE_SIZE, y + TILE_SIZE, LIB_COLOR_RED), 3.0f);

                    engine->Draw2DLine(libVertex(x, y + TILE_SIZE, LIB_COLOR_RED),
                                       libVertex(x + TILE_SIZE, y, LIB_COLOR_RED), 3.0f);
                }
            }

            // Flags
            if (((gameState != LOST) || (gameState == LOST && tile.type == Tile::MINED)) && tile.state == Tile::FLAGGED)
            {
                t_flag->Draw2DQuad(q_tile, x, y);
            }
            // Question marks
            else if (gameState == PLAYING && tile.state == Tile::QUESTIONED)
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
}

/*
===================
Game::Update
===================
*/
void Game::Update()
{
    buttonRestart.Update();

    if (buttonRestart.IsPressed())
        buttonRestart.SetTexture(t_tileOpen);

    if (buttonRestart.IsReleased())
        Restart();

    // Sets the smile button to default state
    if (!engine->IsKey(LIBK_MOUSE_LEFT))
    {
        if (gameState == PLAYING)
            t_curSmile = t_smile;

        buttonRestart.SetTexture(t_tile);
    }

    if (gameState != PLAYING)
        return;

    bool tileShown = false;

    // Tiles
    for (int i = 0; i < fieldSize.x; i++)
    {
        for (int j = 0; j < fieldSize.y; j++)
        {
            Tile &tile = field[i][j];
            tile.button.Update();

            if (tile.CanOpen())
            {
                if (tile.button.IsPressed())
                {
                    tileClicked = true;
                    t_curSmile = t_smileClick;
                }

                // Makes tiles pressed when holding the left mouse button
                if (tile.button.IsHovered() && !tileShown)
                {
                    if (tileClicked && engine->IsKey(LIBK_MOUSE_LEFT))
                    {
                        tileShown = true;
                        tile.button.SetTexture(t_tileOpen);
                    }
                }
                else
                {
                    tile.button.SetTexture(t_tile);
                }

                // Opens a tile
                if (tileClicked && tile.button.IsHovered() && engine->IsKeyReleased(LIBK_MOUSE_LEFT))
                {
                    tileClicked = false;
                    tile.button.SetTexture(t_tileOpen);
                    tile.state = Tile::OPEN;

                    // Mine strike
                    if (tile.type == Tile::MINED)
                    {
                        gameState = LOST;
                        t_curSmile = t_smileLost;
                        boom.Set(tile.button.pos.x, tile.button.pos.y);
                        t_boom->Play();
                        s_boom->Play();
                        tile.button.textureColor.base = LIB_COLOR_RED;
                        tile.button.SetEnabled(false);
                        timer.Stop();
                        ShowAllMines();

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

                    if (tile.HasNoNearestMines())
                        OpenEmptyNeighborTiles();

                    if (!hasUnopenEmptyTiles())
                    {
                        gameState = WON;
                        t_curSmile = t_smileWin;
                        timer.Stop();
                        FlagClosedMinesTiles();

                        // Adjusts the level of difficulty
                        attempts = 0;
                        mineRatio += MINE_RATIO_CHANGE;

                        if (gameTime < PREFERRED_GAME_DURATION)
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
                }
            }

            // Sets/unsets flags and question marks
            if (tile.state != Tile::OPEN && tile.button.IsHovered() && engine->IsKeyPressed(LIBK_MOUSE_RIGHT))
            {
                // Flagged
                if (tile.state == Tile::CLOSED)
                {
                    tile.state = Tile::FLAGGED;
                    shownMinesLeft--;

                    if (tile.type == Tile::MINED)
                        minesLeft--;
                }
                // Question mark
                else if (tile.state == Tile::FLAGGED)
                {
                    tile.state = Tile::QUESTIONED;
                    shownMinesLeft++;
                }
                // Closed empty tile
                else if (tile.state == Tile::QUESTIONED)
                {
                    tile.state = Tile::CLOSED;

                    if (tile.type == Tile::MINED)
                        minesLeft++;
                }
            }
        }
    }

    if (!engine->IsKey(LIBK_MOUSE_LEFT))
        tileClicked = false;
}

/*
===================
Game::Restart
===================
*/
void Game::Restart()
{
    fieldSize = newFieldSize;
    int numOfMines = libCast<int>(fieldSize.x * fieldSize.y * mineRatio);
    libArray<libVec2i> mines;

    for (int i = 0; i < fieldSize.x; i++)
    {
        for (int j = 0; j < fieldSize.y; j++)
        {
            Tile &tile = field[i][j];

            tile.button.SetTexture(t_tile);
            tile.Reset();
        }
    }

    gameTime = 0;
    gameState = PLAYING;
    timer.Restart();
    t_boom->Reset();

    // Gets possible mine locations
    for (int i = 0; i < fieldSize.x * fieldSize.y; i++)
        mines.Append(libVec2i(i % fieldSize.x, i / fieldSize.x));

    // Adds mines to the game field
    for (int i = 0; i < numOfMines; i++)
    {
        if (mines.IsEmpty())
            break;

        int n = libRandom::Int(0, mines.Size() - 1);
        field[mines[n].x][mines[n].y].type = Tile::MINED;
        mines.RemoveIndex(n);
    }

    // Calculates number of nearest mines for each tile
    for (int i = 0; i < fieldSize.x; i++)
    {
        for (int j = 0; j < fieldSize.y; j++)
        {
            if (field[i][j].type == Tile::MINED)
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

                    if (field[i + x][j + y].type == Tile::MINED)
                        mines++;
                }
            }

            field[i][j].nearestMines = mines;
        }
    }

    shownMinesLeft = numOfMines;
    minesLeft = numOfMines;

    int width = fieldSize.x * TILE_SIZE;
    int height = libCast<int>(fieldSize.y * TILE_SIZE + buttonRestart.size.y);
    engine->SetState(LIB_WINDOW_SIZE, width, height);
}

/*
===================
Game::OpenEmptyNeighborTiles
===================
*/
void Game::OpenEmptyNeighborTiles()
{
    for (bool done = false; !done;)
    {
        done = true;

        for (int i = 0; i < fieldSize.x; i++)
        {
            for (int j = 0; j < fieldSize.y; j++)
            {
                Tile &tile = field[i][j];

                if (tile.type == Tile::MINED || tile.state == Tile::OPEN)
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

                        if (field[i + x][j + y].state != Tile::OPEN || field[i + x][j + y].nearestMines)
                            continue;

                        if (tile.state == Tile::FLAGGED)
                            shownMinesLeft++;

                        tile.button.SetTexture(t_tileOpen);
                        tile.state = Tile::OPEN;
                        done = false;
                    }
                }
            }
        }
    }
}

/*
===================
Game::FlagClosedMinesTiles
===================
*/
void Game::FlagClosedMinesTiles()
{
    for (int i = 0; i < fieldSize.x; i++)
    {
        for (int j = 0; j < fieldSize.y; j++)
        {
            Tile &tile = field[i][j];

            if (tile.type != Tile::MINED || tile.state == Tile::OPEN || tile.state == Tile::FLAGGED)
                continue;

            tile.state = Tile::FLAGGED;
            shownMinesLeft--;
        }
    }
}

/*
===================
Game::ShowAllMines
===================
*/
void Game::ShowAllMines()
{
    for (int i = 0; i < fieldSize.x; i++)
    {
        for (int j = 0; j < fieldSize.y; j++)
        {
            Tile &tile = field[i][j];

            if (tile.IsIncorrectlyFlaggedOrMined())
                tile.button.SetTexture(t_tileOpen);
        }
    }
}

/*
===================
Game::ClampFieldDimensions
===================
*/
void Game::ClampFieldDimensions()
{
    if (mineRatio < MINIMAL_MINE_RATIO)
        mineRatio = MINIMAL_MINE_RATIO;
    else if (mineRatio > MAXIMAL_MINE_RATIO)
        mineRatio = MAXIMAL_MINE_RATIO;

    if (newFieldSize.x < MINIMAL_FIELD_WIDTH)
        newFieldSize.x = MINIMAL_FIELD_WIDTH;
    else if (newFieldSize.x > MAXIMAL_FIELD_WIDTH)
        newFieldSize.x = MAXIMAL_FIELD_WIDTH;

    if (newFieldSize.y < MINIMAL_FIELD_HEIGHT)
        newFieldSize.y = MINIMAL_FIELD_HEIGHT;
    else if (newFieldSize.y > MAXIMAL_FIELD_HEIGHT)
        newFieldSize.y = MAXIMAL_FIELD_HEIGHT;
}

/*
===================
Game::hasUnopenEmptyTiles
===================
*/
bool Game::hasUnopenEmptyTiles() const
{
    for (int i = 0; i < fieldSize.x; i++)
        for (int j = 0; j < fieldSize.y; j++)
            if (field[i][j].type == Tile::EMPTY && field[i][j].state != Tile::OPEN)
                return true;

    return false;
}

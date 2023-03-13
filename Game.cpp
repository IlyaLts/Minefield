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
#include "Game.h"

const char *controls = "Mouse:\n"
                       "LMB - Open a tile\n"
                       "RMB - Flag a tile\n"
                       "LMB + RMB - Chord\n"
                       "MMB - Chord\n"
                       "\n"
                       "Keyboard:\n"
                       "Space - Flag a tile\n"
                       "Ctrl - Open a tile\n"
                       "Shift - Chord\n"
                       "Ctrl + Space - Chord\n"
                       "F1 - Help\n"
                       "F2 - Controls\n"
                       "F3 - Settings\n"
                       "F4 - Turn on/off sound\n"
                       "F12 - Take a screenshot";

const char *help = "The rules are simple: click on a tile to reveal what's underneath.\n"
                   "If it's a mine, you lose. If it's a number, it tells you how many mines\n"
                   "are adjacent to that tile. If it's blank, it means there are no mines nearby\n"
                   "and the surrounding tiles are automatically revealed. You can also right - click\n"
                   "on a tile to mark it as a mine or as uncertain. Additionally, you can use the chording\n"
                   "feature, by middle-clicking a numbered tile to reveal its surrounding tiles, but\n"
                   "only if you've correctly flagged the exact number of adjacent mines. Be careful,\n"
                   "as incorrectly flagged mines will cause you to lose.\n"
                   "The goal is to clear all the tiles that don't have mines.\n\n"
                   "Copyright (C) 2023-2025 Ilya Lyakhovets";

/*
===================
Game::Init
===================
*/
bool Game::Init()
{
    engine->Get(mesh_smile.Get());
    engine->Get(mesh_scoreboard.Get());
    engine->Get(mesh_tile.Get());
    engine->Get(mesh_tileOpen.Get());
    engine->Get(mesh_mine.Get());
    engine->Get(mesh_question.Get());
    engine->Get(mesh_flag.Get());
    engine->Get(mesh_panel.Get());
    engine->Get(mesh_lines.Get());

    LIB_CHECK(engine->Get(tex_panel.Get(), DATA_PACK "Textures/Panel.tga"));
    LIB_CHECK(engine->Get(tex_scoreboard.Get(), DATA_PACK "Textures/Scoreboard.tga"));
    LIB_CHECK(engine->Get(tex_smile.Get(), DATA_PACK "Textures/Smile.tga"));
    LIB_CHECK(engine->Get(tex_smileClick.Get(), DATA_PACK "Textures/SmileClick.tga"));
    LIB_CHECK(engine->Get(tex_smileWin.Get(), DATA_PACK "Textures/SmileWon.tga"));
    LIB_CHECK(engine->Get(tex_smileLost.Get(), DATA_PACK "Textures/SmileLost.tga"));
    LIB_CHECK(engine->Get(tex_tile.Get(), DATA_PACK "Textures/Tile.tga"));
    LIB_CHECK(engine->Get(tex_tileOpen.Get(), DATA_PACK "Textures/TileOpen.tga"));
    LIB_CHECK(engine->Get(tex_mine.Get(), DATA_PACK "Textures/Mine.tga"));
    LIB_CHECK(engine->Get(tex_flag.Get(), DATA_PACK "Textures/Flag.tga"));
    LIB_CHECK(engine->Get(tex_question.Get(), DATA_PACK "Textures/Question.tga"));
    LIB_CHECK(engine->Get(spr_boom.Get(), DATA_PACK "Textures/Boom/Boom.tga"));
    LIB_CHECK(engine->Get(font.Get(), DATA_PACK "Font.ttf"));
    LIB_CHECK(engine->Get(digital.Get(), DATA_PACK "Digital.ttf"));
    LIB_CHECK(engine->Get(snd_boom.Get(), DATA_PACK "Sounds/Boom.wav"));

    font->SetAlign(LIB_CENTER);
    font->SetShadowType(libFont::SHADOW_ADDAPTIVE);
    font->SetShadowShift(libVec2(1.0f, 1.0f));

    digital->SetSize(28);
    digital->SetAlign(LIB_CENTER);

    spr_boom->SetDuration(BOOM_DURATION);
    spr_boom->SetStyle(libSprite::ONCE);

    tex_curSmile = tex_smile;

    buttonRestart.SetSize(TILE_SIZE * 2, TILE_SIZE * 2);
    buttonRestart.SetTexture(tex_tile.Get());

    buttonSettings.SetSize(TILE_SIZE * 2, TILE_SIZE / 2);
    buttonSettings.SetFont(font.Get());
    buttonSettings.SetTexture(tex_tile.Get());
    buttonSettings.SetTextScale(1.4f);
    //buttonSettings.SetText(L". . .");

    libStr path;
    libDir::GetLocalDataLocation(path);
    path.Append("/Minefield");
    libDir::Create(path.Get());
    path.Append("/Minefield.cfg");

    cfg.Load(path.Get(), DATA_PACK "Minefield.cfg");
    mineRatio = cfg.GetFloat("AutoMineRatio", DEFAULT_MINE_RATIO);
    autoFieldSize.x = cfg.GetInt("AutoFieldWidth", DEFAULT_AUTO_FIELD_WIDTH);
    autoFieldSize.y = cfg.GetInt("AutoFieldHeight", DEFAULT_AUTO_FIELD_HEIGHT);

    engine->SetState(LIB_AUDIO_VOLUME, cfg.GetFloat("AudioVolume", DEFAULT_AUDIO_VOLUME));

    LIB_CHECK(settings.Init());

    ClampFieldDimensions();
    Restart();
    UpdatePanelsMesh();

    return true;
}

/*
===================
Game::Draw
===================
*/
void Game::Draw()
{
    if (updateTilesMesh)
    {
        updateTilesMesh = false;
        UpdateTilesMesh();
    }

    libVec2i screenSize(engine->State(LIB_SCREEN_WIDTH), engine->State(LIB_SCREEN_HEIGHT));

    engine->Draw(mesh_panel.Get(), tex_panel.Get(), true);

    if (settingsShown)
    {
        settings.Draw();
        return;
    }

    float halfTile = TILE_SIZE / 2.0f;
    libQuad q_tile(libVertex(0.0f, 0.0f, 0.0f, 0.0f), libVertex(TILE_SIZE, TILE_SIZE, 1.0f, 1.0f));

    libVec2i upperPanelSize(screenSize.x - MARGIN_X * 2, TILE_SIZE * 2 + TILE_SIZE);
    libVec2i pOffset(MARGIN_X * 2, MARGIN_Y * 2);
    libVec2i p2Offset(MARGIN_X * 2, upperPanelSize.y + MARGIN_Y * 5);

    buttonRestart.SetPosition(screenSize.x / 2.0f, pOffset.y + buttonRestart.size.y / 2.0f + halfTile / 2.0f);
    buttonRestart.Draw();

    buttonSettings.SetPosition(screenSize.x / 2.0f, pOffset.y + buttonSettings.size.y / 2.0f + TILE_SIZE * 2.0f + halfTile / 2.0f);
    buttonSettings.Draw();

    // ...
    auto shadowType = font->ShadowType();
    font->SetColor(LIB_COLOR_BLACK);
    font->SetSize(libCast<int>(TILE_SIZE / 2) - 1);
    font->SetShadowType(libFont::NO_SHADOW);
    font->Print2D(screenSize.x / 2.0f, pOffset.y + buttonRestart.size.y + buttonSettings.size.y / 2.0f, ". . .");
    font->SetShadowType(shadowType);

    if (buttonRestart.IsPressed())
        mesh_smile->scale.Set(0.9f, 0.8f, 1.0f);
    else
        mesh_smile->scale.Set(1.0f, 1.0f, 1.0f);

    engine->Draw(mesh_smile.Get(), tex_curSmile.Get(), true);
    engine->Draw(mesh_scoreboard.Get(), tex_scoreboard.Get(), true);
    engine->Draw(mesh_tile.Get(), tex_tile.Get(), true);
    engine->Draw(mesh_tileOpen.Get(), tex_tileOpen.Get(), true);
    engine->Draw(mesh_mine.Get(), tex_mine.Get(), true);
    engine->Draw(mesh_question.Get(), tex_question.Get(), true);
    engine->Draw(mesh_flag.Get(), tex_flag.Get(), true);
    engine->Draw(mesh_lines.Get(), nullptr, true);

    // Scoreboards
    libVec2 scoreboardSize(TILE_SIZE * 1.5f, TILE_SIZE * 0.9f);
    libVec2 sbPos(pOffset.x + halfTile + scoreboardSize.x, pOffset.y + TILE_SIZE + halfTile);
    libVec2 sbPos2(-pOffset.y + screenSize.x - halfTile - scoreboardSize.x, pOffset.y + TILE_SIZE + halfTile);

    // The font doesn't appear to be exactly in the center, so this corrects that
    float digitalFontOffset = -1.0f;
    int printableMinesLeft = shownMinesLeft;

    if (printableMinesLeft < SCOREBOARD_MIN_VALUE)
        printableMinesLeft = SCOREBOARD_MIN_VALUE;

    if (printableMinesLeft > SCOREBOARD_MAX_VALUE)
        printableMinesLeft = SCOREBOARD_MAX_VALUE;

    digital->SetColor(libColor(0.25f, 0.0f, 0.0f));
    digital->Print2D(sbPos.x, sbPos.y + digitalFontOffset, "%03d", 888);
    digital->Print2D(sbPos2.x, sbPos2.y + digitalFontOffset, "%03d", 888);

    digital->SetColor(LIB_COLOR_RED);
    digital->Print2D(sbPos.x, sbPos.y + digitalFontOffset, "%03d", printableMinesLeft);
    digital->Print2D(sbPos2.x, sbPos2.y + digitalFontOffset, "%03d", gameTime > SCOREBOARD_MAX_VALUE ? SCOREBOARD_MAX_VALUE : gameTime);

    for (int i = 0; i < fieldSize.x; i++)
    {
        for (int j = 0; j < fieldSize.y; j++)
        {
            float x = p2Offset.x + libCast<float>(TILE_SIZE * i) - 1; // Minus 1 for fixing a small gap on the left side
            float y = p2Offset.y + libCast<float>(TILE_SIZE * j);
            Tile &tile = field[i][j];

            // Number of the nearest mines around a tile
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
                    font->Print2D(x + halfTile, y + halfTile, "%d", tile.nearestMines);
                }
            }
        }
    }

    // Mine explosion
    if (gameState == LOST && spr_boom->IsPlaying())
    {
        libQuad q_boom(libVertex(-BOOM_RATIUS, -BOOM_RATIUS, 0.0f, 0.0f), libVertex(BOOM_RATIUS, BOOM_RATIUS, 1.0f, 1.0f));
        spr_boom->Draw2DQuad(q_boom, boomCoord.x, boomCoord.y);
    }
}

/*
===================
Game::Update
===================
*/
void Game::Update()
{
    if (engine->IsKeyPressed(LIBK_F1))
        ShowHelp();
    
    if (engine->IsKeyPressed(LIBK_F2))
        Restart();

    if (engine->IsKeyPressed(LIBK_F3))
        ToggleSettings();

    if (engine->IsKeyPressed(LIBK_F4))
        ToggleAudio();

    if (settingsShown)
    {
        settings.Update();
        return;
    }

    gameTime = libCast<int>(timer.Seconds());

    buttonRestart.Update();
    buttonSettings.Update();

    if (buttonRestart.IsPressed())
        buttonRestart.SetTexture(tex_tileOpen.Get());
    else if (buttonRestart.IsReleased())
        Restart();
    else
        buttonRestart.SetTexture(tex_tile.Get());

    if (buttonSettings.IsPressed())
        buttonSettings.SetTexture(tex_tileOpen.Get());
    else if (buttonSettings.IsReleased())
        ToggleSettings();
    else
        buttonSettings.SetTexture(tex_tile.Get());
        
    if (gameState == PLAYING)
        UpdateTiles();

    // Sets the smile button to its default state when a tile is not being pressed
    if (gameState == PLAYING)
        if (!LeftPressing() && !MiddlePressing())
            tex_curSmile = tex_smile;
}

/*
===================
Game::Restart
===================
*/
void Game::Restart()
{
    if (settings.Difficulty() == Settings::BEGINNER)
    {
        fieldSize.Set(10, 10);
        minesLeft = 10;
    }
    else if (settings.Difficulty() == Settings::INTERMEDIATE)
    {
        fieldSize.Set(16, 16);
        minesLeft = 40;
    }
    else if (settings.Difficulty() == Settings::EXPERT)
    {
        fieldSize.Set(30, 16);
        minesLeft = 99;
    }
    else if (settings.Difficulty() == Settings::AUTO)
    {
        fieldSize = autoFieldSize;
        minesLeft = libCast<int>(fieldSize.x * fieldSize.y * mineRatio);
    }
    else if (settings.Difficulty() == Settings::CUSTOM)
    {
        fieldSize.Set(settings.CustomWidth(), settings.CustomHeight());
        minesLeft = settings.CustomMines();
    }

    if (minesLeft >= fieldSize.x * fieldSize.y)
        minesLeft = fieldSize.x * fieldSize.y - 1;

    for (int i = 0; i < fieldSize.x; i++)
    {
        for (int j = 0; j < fieldSize.y; j++)
        {
            Tile &tile = field[i][j];

            tile.button.SetTexture(tex_tile.Get());
            tile.Reset();
        }
    }

    gameTime = 0;
    gameState = PLAYING;
    timer.Reset();
    spr_boom->Reset();
    firstClick = true;
    shownMinesLeft = minesLeft;
    AdjustWindowSize();
    UpdatePanelsMesh();
    updateTilesMesh = true;
}

/*
===================
Game::SaveSettings
===================
*/
void Game::SaveSettings()
{
    cfg.SetFloat("AudioVolume", engine->State(LIB_AUDIO_VOLUME));
    cfg.SetFloat("AutoMineRatio", mineRatio);
    cfg.SetInt("AutoFieldWidth", autoFieldSize.x);
    cfg.SetInt("AutoFieldHeight", autoFieldSize.y);

    cfg.Save();
}

/*
===================
Game::ShowHelp
===================
*/
void Game::ShowHelp() const
{
    libDialog::Information(libFormat("Minefield Version: %s", MINEFIELD_VERSION), help);
    libDialog::Information(libFormat("Minefield Controls:"), controls);
}

/*
===================
Game::ToggleSettings
===================
*/
void Game::ToggleSettings()
{
    settingsShown = !settingsShown;
    AdjustWindowSize();
    UpdatePanelsMesh();
}

/*
===================
Game::ToggleAudio
===================
*/
void Game::ToggleAudio()
{
    if (engine->State(LIB_AUDIO_VOLUME))
        engine->SetState(LIB_AUDIO_VOLUME, 0.0f);
    else
        engine->SetState(LIB_AUDIO_VOLUME, DEFAULT_AUDIO_VOLUME);
}

/*
===================
Game::UpdatePanelsMesh
===================
*/
void Game::UpdatePanelsMesh()
{
    mesh_panel->Clear();
    libVec2i screenSize(engine->State(LIB_SCREEN_WIDTH), engine->State(LIB_SCREEN_HEIGHT));

    if (settingsShown)
    {
        AddPanelMesh(libVec2(MARGIN_X, MARGIN_Y), libVec2(libCast<float>(screenSize.x - MARGIN_X), libCast<float>(screenSize.y - MARGIN_Y)), 25.0f);
    }
    else
    {
        libVec2i upperPanelSize(screenSize.x - MARGIN_X * 2, TILE_SIZE * 2 + TILE_SIZE);
        libVec2i fieldTrueSize = fieldSize * TILE_SIZE;

        AddPanelMesh(libVec2(MARGIN_X, MARGIN_Y), libVec2(libCast<float>(upperPanelSize.x + MARGIN_X), upperPanelSize.y + MARGIN_Y * 2.0f), 25.0f);
        AddPanelMesh(libVec2(MARGIN_X, upperPanelSize.y + MARGIN_Y * 4.0f), libVec2(fieldTrueSize.x + MARGIN_X * 3.0f, upperPanelSize.y + fieldTrueSize.y + MARGIN_Y * 6.0f), 70.0f);

        // Scoreboards
        libVec2 scoreboardSize(TILE_SIZE * 1.5f, TILE_SIZE * 0.9f);
        float halfTile = TILE_SIZE / 2.0f;
        libVec2i pOffset(MARGIN_X * 2, MARGIN_Y * 2);
        libVec2 sbPos(pOffset.x + halfTile + scoreboardSize.x, pOffset.y + TILE_SIZE + halfTile);
        libVec2 sbPos2(-pOffset.y + screenSize.x - halfTile - scoreboardSize.x, pOffset.y + TILE_SIZE + halfTile);

        libQuad q_scoreboard(libVertex(-scoreboardSize.x, -scoreboardSize.y, 0.0f, 0.0f), libVertex(scoreboardSize.x, scoreboardSize.y, 1.0f, 1.0f));
        mesh_scoreboard->Clear();
        mesh_scoreboard->Add(q_scoreboard, libVec3(sbPos.x, sbPos.y, 0.0f));
        mesh_scoreboard->Add(q_scoreboard, libVec3(sbPos2.x, sbPos2.y, 0.0f));

        // Smile
        mesh_smile->Clear();
        libQuad q_smile(libVertex(-TILE_SIZE, -TILE_SIZE, 0.0f, 0.0f), libVertex(TILE_SIZE, TILE_SIZE, 1.0f, 1.0f));
        mesh_smile->Add(q_smile);
        mesh_smile->position = libVec3(screenSize.x / 2.0f, pOffset.y + TILE_SIZE + halfTile / 2.0f, 0.0f);
    }
}

/*
===================
Game::UpdateTilesMesh
===================
*/
void Game::UpdateTilesMesh()
{
    libVec2i screenSize(engine->State(LIB_SCREEN_WIDTH), engine->State(LIB_SCREEN_HEIGHT));
    float halfTile = TILE_SIZE / 2.0f;
    libQuad q_tile(libVertex(0.0f, 0.0f, 0.0f, 0.0f), libVertex(TILE_SIZE, TILE_SIZE, 1.0f, 1.0f));
    libVec2i upperPanelSize(screenSize.x - MARGIN_X * 2, TILE_SIZE * 2 + TILE_SIZE);
    libVec2i p2Offset(MARGIN_X * 2, upperPanelSize.y + MARGIN_Y * 5);

    mesh_tile->Clear();
    mesh_tileOpen->Clear();
    mesh_mine->Clear();
    mesh_question->Clear();
    mesh_flag->Clear();
    mesh_lines->Clear();

    // Field
    for (int i = 0; i < fieldSize.x; i++)
    {
        for (int j = 0; j < fieldSize.y; j++)
        {
            float x = p2Offset.x + libCast<float>(TILE_SIZE * i) - 1; // Minus 1 for fixing a small gap on the left side
            float y = p2Offset.y + libCast<float>(TILE_SIZE * j);
            Tile &tile = field[i][j];

            // Tiles
            tile.button.SetSize(TILE_SIZE, TILE_SIZE);
            tile.button.SetPosition(x + libMath::Ceil(halfTile), y + libMath::Ceil(halfTile));

            if (tile.button.texture == tex_tileOpen.Get())
            {
                q_tile.SetColor(tile.button.textureColor.base);
                mesh_tileOpen->Add(q_tile, libVec3(x, y, 0.0f));
                q_tile.SetColor(LIB_COLOR_WHITE);
            }
            else
            {
                mesh_tile->Add(q_tile, libVec3(x, y, 0.0f));
            }

            // Mines
            if (gameState == LOST && tile.IsIncorrectlyFlaggedOrMined())
            {
                mesh_mine->Add(q_tile, libVec3(x, y, 0.0f));

                // Cross out, which indicates wrongly placed flags
                if (tile.type == Tile::EMPTY && tile.state == Tile::FLAGGED)
                {
                    engine->DrawLine(mesh_lines.Get(), libVertex(x, y, LIB_COLOR_RED),
                                     libVertex(x + TILE_SIZE, y + TILE_SIZE, LIB_COLOR_RED), 3.0f);

                    engine->DrawLine(mesh_lines.Get(), libVertex(x, y + TILE_SIZE, LIB_COLOR_RED),
                                     libVertex(x + TILE_SIZE, y, LIB_COLOR_RED), 3.0f);
                }
            }

            // Flags
            if ((gameState != LOST || tile.type == Tile::MINED) && tile.state == Tile::FLAGGED)
            {
                mesh_flag->Add(q_tile, libVec3(x, y, 0.0f));
            }
            // Question marks
            else if (gameState == PLAYING && tile.state == Tile::QUESTIONED)
            {
                mesh_question->Add(q_tile, libVec3(x, y, 0.0f));
            }
        }
    }
}

/*
===================
Game::AddPanelMesh
===================
*/
void Game::AddPanelMesh(const libVec2 corner, const libVec2 &corner2, float thickness)
{
    libQuad q_panel;
    libVec2 size = corner2 - corner;

    // Left upper corner
    q_panel.Set(libVertex(0.0f, 0.0f, 0.0f, 0.0f), libVertex(thickness, thickness, 0.25f, 0.25f));
    mesh_panel->Add(q_panel, libVec3(corner.x, corner.y, 0.0f));

    // Right upper corner
    q_panel.Set(libVertex(-thickness, 0.0f, 0.75f, 0.0f), libVertex(0.0f, thickness, 1.0f, 0.25f));
    mesh_panel->Add(q_panel, libVec3(corner2.x, corner.y, 0.0f));

    // Left lower corner
    q_panel.Set(libVertex(0.0f, -thickness, 0.0f, 0.75f), libVertex(thickness, 0.0f, 0.25f, 1.0f));
    mesh_panel->Add(q_panel, libVec3(corner.x, corner2.y, 0.0f));

    // Right lower corner
    q_panel.Set(libVertex(-thickness, -thickness, 0.75f, 0.75f), libVertex(0.0f, 0.0f, 1.0f, 1.0f));
    mesh_panel->Add(q_panel, libVec3(corner2.x, corner2.y, 0.0f));

    // Left side
    q_panel.Set(libVertex(0.0f, thickness, 0.0f, 0.25f), libVertex(thickness, size.y - thickness, 0.25f, 0.75f));
    mesh_panel->Add(q_panel, libVec3(corner.x, corner.y, 0.0f));

    // Upper side
    q_panel.Set(libVertex(thickness, 0.0f, 0.25f, 0.0f), libVertex(size.x + 5.0f - thickness, thickness, 0.75f, 0.25f));
    mesh_panel->Add(q_panel, libVec3(corner.x, corner.y, 0.0f));

    // Right side
    q_panel.Set(libVertex(-thickness, thickness, 0.75f, 0.25f), libVertex(0.0f, size.y - thickness, 1.0f, 0.75f));
    mesh_panel->Add(q_panel, libVec3(corner2.x, corner.y, 0.0f));

    // Bottom side
    q_panel.Set(libVertex(thickness, -thickness, 0.25f, 0.75f), libVertex(size.x + 5.0f - thickness, 0.0f, 0.75f, 1.0f));
    mesh_panel->Add(q_panel, libVec3(corner.x, corner2.y, 0.0f));
}

/*
===================
Game::GenerateMines
===================
*/
void Game::GenerateMines()
{
    libArray<libVec2i> mines;
    firstClick = false;

    // Gets possible mine locations
    for (int i = 0; i < fieldSize.x * fieldSize.y; i++)
    {
        libVec2i location(i % fieldSize.x, i / fieldSize.x);

        // There should be no mines in adjacent tiles if the number of mines is 9 fewer than the total number of tiles
        if (fieldSize.x * fieldSize.y - minesLeft >= 9)
        {
            bool skip = false;

            for (int j = firstClickCoord.x - 1; j <= firstClickCoord.x + 1; j++)
                for (int k = firstClickCoord.y - 1; k <= firstClickCoord.y + 1; k++)
                    if (location == libVec2i(j, k))
                        skip = true;

            if (skip)
                continue;
        }
        // Otherwise, no mine on the first clicked tile
        else if (location == firstClickCoord)
        {
            continue;
        }

        mines.Append(location);
    }

    // Adds mines to the game field
    for (int i = 0; i < minesLeft; i++)
    {
        if (mines.IsEmpty())
            break;

        int n = libRandom::Int(0, libCast<int>(mines.Size()) - 1);
        field[mines[n].x][mines[n].y].type = Tile::MINED;
        mines.RemoveIndex(n);
    }

    // Calculates the number of nearest mines for each tile
    for (int i = 0; i < fieldSize.x; i++)
    {
        for (int j = 0; j < fieldSize.y; j++)
        {
            if (field[i][j].type == Tile::MINED)
                continue;

            int minesCount = 0;

            for (int x = -1; x <= 1; x++)
            {
                for (int y = -1; y <= 1; y++)
                {
                    if (i == i + x && j == j + y)
                        continue;

                    if (i + x < 0 || i + x >= fieldSize.x || j + y < 0 || j + y >= fieldSize.y)
                        continue;

                    if (field[i + x][j + y].type == Tile::MINED)
                        minesCount++;
                }
            }

            field[i][j].nearestMines = minesCount;
        }
    }
}

/*
===================
Game::UpdateHoveredTile
===================
*/
void Game::UpdateHoveredTile()
{
    for (int i = 0; i < fieldSize.x; i++)
    {
        for (int j = 0; j < fieldSize.y; j++)
        {
            if (field[i][j].button.IsHovered())
            {
                hoveredTile = true;
                hoveredTileCoord.Set(i, j);
                return;
            }
        }
    }

    hoveredTile = false;
    hoveredTileCoord.Set(-1, -1);
}

/*
===================
Game::UpdateTiles
===================
*/
void Game::UpdateTiles()
{
    UpdateHoveredTile();

    for (int i = 0; i < fieldSize.x; i++)
    {
        for (int j = 0; j < fieldSize.y; j++)
        {
            Tile &tile = field[i][j];
            tile.button.Update();

            // Initiates tile pressing only if it was pressed from the beginning
            if (IsTileHovered(i, j) && (LeftPressed() || MiddlePressed()))
            {
                tileClicked = true;
                tex_curSmile = tex_smileClick;
            }

            if (!LeftPressed() && LeftPressing() && RightPressed())
            {
                tileClicked = true;
                tex_curSmile = tex_smileClick;
            }

            // Makes tiles pressed/unpressed when holding the left mouse button
            if (IsTileHovered(i, j) && tileClicked)
            {
                if (LeftPressing())
                {
                    // Actually makes the hovered tile pressed
                    if (tile.CanOpen() && tile.button.texture != tex_tileOpen.Get())
                    {
                        tile.button.SetTexture(tex_tileOpen.Get());
                        updateTilesMesh = true;
                    }

                    // Starts chording with left/right mouse clicks
                    if (RightPressing())
                        SetNeighborPressState(i, j, true);
                }

                // Starts chording with the wheel button click
                if (MiddlePressing())
                    SetNeighborPressState(i, j, true);
            }

            if (tile.CanOpen() && IsTileToBeUnpressed(i, j) && tile.button.texture != tex_tile.Get())
            {
                tile.button.SetTexture(tex_tile.Get());
                updateTilesMesh = true;
            }

            // Detects chording or opening the hovered tile
            if ((LeftPressing() || tileClicked) && IsTileHovered(i, j))
            {
                if (((LeftReleased() || RightReleased()) && (LeftPressing() || RightPressing())) || MiddleReleased())
                {
                    Chord(i, j);
                    SetNeighborPressState(i, j, false);
                }
                else if ((LeftReleased() && !RightPressing()))
                {
                    OpenTile(i, j);
                }

                // Fixes unrevealed tiles containing mines when losing
                if (gameState != PLAYING)
                    return;
            }
        }
    }

    // Avoids opening a tile after triggering chording 
    if (LeftPressing() && (RightReleased() || MiddleReleased()))
        tileClicked = false;

    if (!LeftPressing() && !MiddlePressing())
        tileClicked = false;

    UpdateTileFlags();
}

/*
===================
Game::UpdateTileFlags

Sets/unsets flags and question marks
===================
*/
void Game::UpdateTileFlags()
{
    // Unpress tiles and avoid flagging the hovered tile while chording
    if (MiddlePressing() && RightPressed())
    {
        tileClicked = false;
        return;
    }

    // Do not flag a tile if that tile has already been pressed
    if (LeftPressing() || !RightPressed())
        return;

    for (int i = 0; i < fieldSize.x; i++)
    {
        for (int j = 0; j < fieldSize.y; j++)
        {
            Tile &tile = field[i][j];

            if (tile.state == Tile::OPEN || !IsTileHovered(i, j))
                continue;

            // Flagged
            if (tile.state == Tile::CLOSED)
            {
                tile.state = Tile::FLAGGED;
                shownMinesLeft--;

                if (tile.type == Tile::MINED)
                    minesLeft--;

                updateTilesMesh = true;
            }
            // Question mark
            else if (tile.state == Tile::FLAGGED)
            {
                if (settings.MarksEnabled())
                    tile.state = Tile::QUESTIONED;
                else
                    tile.state = Tile::CLOSED;

                shownMinesLeft++;
                updateTilesMesh = true;
            }
            // Closed empty tile
            else if (tile.state == Tile::QUESTIONED)
            {
                tile.state = Tile::CLOSED;

                if (tile.type == Tile::MINED)
                    minesLeft++;

                updateTilesMesh = true;
            }

            return;
        }
    }
}

/*
===================
Game::LeftPressed
===================
*/
bool Game::LeftPressed() const
{
    return engine->IsKeyPressed(LIBK_MOUSE_LEFT) || engine->IsKeyPressed(LIBK_CTRL);
}

/*
===================
Game::LeftReleased
===================
*/
bool Game::LeftReleased() const
{
    return engine->IsKeyReleased(LIBK_MOUSE_LEFT) || engine->IsKeyReleased(LIBK_CTRL);
}

/*
===================
Game::LeftPressing
===================
*/
bool Game::LeftPressing() const
{
    return engine->IsKey(LIBK_MOUSE_LEFT) || engine->IsKey(LIBK_CTRL);
}

/*
===================
Game::RightPressed
===================
*/
bool Game::RightPressed() const
{
    return engine->IsKeyPressed(LIBK_MOUSE_RIGHT) || engine->IsKeyPressed(LIBK_SPACE);
}

/*
===================
Game::RightReleased
===================
*/
bool Game::RightReleased() const
{
    return engine->IsKeyReleased(LIBK_MOUSE_RIGHT) || engine->IsKeyReleased(LIBK_SPACE);
}

/*
===================
Game::RightPressing
===================
*/
bool Game::RightPressing() const
{
    return engine->IsKey(LIBK_MOUSE_RIGHT) || engine->IsKey(LIBK_SPACE);
}

/*
===================
Game::MiddlePressed
===================
*/
bool Game::MiddlePressed() const
{
    return engine->IsKeyPressed(LIBK_MOUSE_MIDDLE) || engine->IsKeyPressed(LIBK_SHIFT);
}

/*
===================
Game::MiddleReleased
===================
*/
bool Game::MiddleReleased() const
{
    return engine->IsKeyReleased(LIBK_MOUSE_MIDDLE) || engine->IsKeyReleased(LIBK_SHIFT);
}

/*
===================
Game::MiddlePressing
===================
*/
bool Game::MiddlePressing() const
{
    return engine->IsKey(LIBK_MOUSE_MIDDLE) || engine->IsKey(LIBK_SHIFT);
}

/*
===================
Game::OpenTile
===================
*/
void Game::OpenTile(int x, int y)
{
    firstClickCoord.Set(x, y);
    Tile &tile = field[x][y];

    // Unpress tiles and avoid opening the hovered tile while chording
    if (MiddlePressing() && LeftReleased())
    {
        tileClicked = false;
        return;
    }

    if (!tile.CanOpen())
        return;

    if (firstClick)
        GenerateMines();

    timer.Start();
    tileClicked = false;
    tile.button.SetTexture(tex_tileOpen.Get());
    tile.state = Tile::OPEN;

    // Game over - mine explosion 
    if (tile.type == Tile::MINED)
    {
        gameState = LOST;
        tex_curSmile = tex_smileLost;
        boomCoord.Set(tile.button.pos.x, tile.button.pos.y);
        spr_boom->Play();
        snd_boom->Play();
        tile.button.textureColor.base = LIB_COLOR_RED;
        tile.button.SetEnabled(false);
        timer.Stop();
        ShowAllMines();

        // Adjusts the difficulty level
        if (settings.Difficulty() == Settings::AUTO)
        {
            attempts++;

            if (attempts >= ATTEMPTS_BEFORE_CHANGE)
            {
                attempts = 0;
                mineRatio -= MINE_RATIO_CHANGE;

                if (mineRatio < MINIMAL_MINE_RATIO)
                    mineRatio = MINIMAL_MINE_RATIO;
            }
        }

        updateTilesMesh = true;
        return;
    }

    if (tile.HasNoNearestMines())
        OpenEmptyNeighborTiles();

    if (!hasUnopenEmptyTiles())
    {
        gameState = WON;
        tex_curSmile = tex_smileWin;
        timer.Stop();
        FlagClosedMineTiles();

        // Adjusts the difficulty level
        if (settings.Difficulty() == Settings::AUTO)
        {
            attempts = 0;
            mineRatio += MINE_RATIO_CHANGE;

            if (gameTime < PREFERRED_GAME_DURATION)
            {
                autoFieldSize.x++;
                autoFieldSize.y++;
            }
            else
            {
                autoFieldSize.x--;
                autoFieldSize.y--;
            }
        }

        ClampFieldDimensions();
    }

    updateTilesMesh = true;
}

/*
====================
Game::Chord

This allows to quickly reveal adjacent tiles if the number of mines matches the number of flags.
====================
*/
void Game::Chord(int x, int y)
{
    if (field[x][y].state != Tile::OPEN)
        return;

    // A chord is allowed only if we have at least one mine
    if (!field[x][y].nearestMines)
        return;

    tileClicked = false;
    int flags = 0;

    // Counts the number of flags
    for (int i = x - 1; i <= x + 1; i++)
    {
        for (int j = y - 1; j <= y + 1; j++)
        {
            if (i < 0 || j < 0)
                continue;

            if (i >= fieldSize.x || j >= fieldSize.y)
                continue;

            if (i == x && j == y)
                continue;

            if (field[i][j].state == Tile::FLAGGED)
                flags++;
        }
    }

    if (flags != field[x][y].nearestMines)
        return;

    // Opens adjacent tiles
    for (int i = x - 1; i <= x + 1; i++)
    {
        for (int j = y - 1; j <= y + 1; j++)
        {
            if (i < 0 || j < 0)
                continue;

            if (i >= fieldSize.x || j >= fieldSize.y)
                continue;

            OpenTile(i, j);
        }
    }
}

/*
===================
Game::SetNeighborPressState

Sets the press state for the hovered tile and the eight tiles around it.
===================
*/
void Game::SetNeighborPressState(int x, int y, bool pressed)
{
    for (int i = x - 1; i <= x + 1; i++)
    {
        for (int j = y - 1; j <= y + 1; j++)
        {
            if (i < 0 || j < 0)
                continue;

            if (i >= fieldSize.x || j >= fieldSize.y)
                continue;

            Tile &tile = field[i][j];

            if (!tile.CanOpen())
                continue;

            if (pressed)
                tile.button.SetTexture(tex_tileOpen.Get());
            else
                tile.button.SetTexture(tex_tile.Get());
        }
    }

    updateTilesMesh = true;
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

                        tile.button.SetTexture(tex_tileOpen.Get());
                        tile.state = Tile::OPEN;
                        done = false;
                    }
                }
            }
        }
    }

    updateTilesMesh = true;
}

/*
===================
Game::FlagClosedMineTiles
===================
*/
void Game::FlagClosedMineTiles()
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

    updateTilesMesh = true;
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
                tile.button.SetTexture(tex_tileOpen.Get());
        }
    }

    updateTilesMesh = true;
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

    if (autoFieldSize.x < MINIMAL_FIELD_WIDTH)
        autoFieldSize.x = MINIMAL_FIELD_WIDTH;
    else if (autoFieldSize.x > MAXIMAL_FIELD_WIDTH)
        autoFieldSize.x = MAXIMAL_FIELD_WIDTH;

    if (autoFieldSize.y < MINIMAL_FIELD_HEIGHT)
        autoFieldSize.y = MINIMAL_FIELD_HEIGHT;
    else if (autoFieldSize.y > MAXIMAL_FIELD_HEIGHT)
        autoFieldSize.y = MAXIMAL_FIELD_HEIGHT;
}

/*
===================
Game::AdjustWindowSize
===================
*/
void Game::AdjustWindowSize()
{
    if (settingsShown)
    {
        int width = 10 * TILE_SIZE;
        int height = libCast<int>(10 * TILE_SIZE + buttonRestart.size.y + TILE_SIZE);

        width += MARGIN_X * 4;
        height += MARGIN_Y * 7;

        engine->SetState(LIB_WINDOW_SIZE, width, height);
    }
    else
    {
        int width = fieldSize.x * TILE_SIZE;
        int height = libCast<int>(fieldSize.y * TILE_SIZE + buttonRestart.size.y + TILE_SIZE);

        width += MARGIN_X * 4;
        height += MARGIN_Y * 7;

        engine->SetState(LIB_WINDOW_SIZE, width, height);
    }
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

/*
===================
Game::IsTileToBeUnpressed
===================
*/
bool Game::IsTileToBeUnpressed(int x, int y) const
{
    if (!tileClicked)
        return true;

    if ((!LeftPressing() || !IsTileHovered(x, y)) && !RightPressing() && !MiddlePressing())
        return true;

    if ((RightPressing() || MiddlePressing()) && !IsAdjacentTileHovered(x, y))
        return true;

    return false;
}

/*
===================
Game::IsAdjacentTileHovered
===================
*/
bool Game::IsAdjacentTileHovered(int x, int y) const
{
    if (!hoveredTile)
        return false;

    if (x + 1 < hoveredTileCoord.x || x - 1 > hoveredTileCoord.x || y + 1 < hoveredTileCoord.y || y - 1 > hoveredTileCoord.y)
        return false;

    return true;
}

/*
===================
Game::IsTileHovered
===================
*/
bool Game::IsTileHovered(int x, int y) const
{
    return hoveredTile && hoveredTileCoord.x == x && hoveredTileCoord.y == y;
}

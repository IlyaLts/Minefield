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

#include "Settings.h"
#include "Game.h"

const wchar_t *difficultyLevels[DIFFICULTY_LEVELS] = { L"Beginner", L"Intermediate", L"Expert", L"Auto", L"Custom" };

/*
===================
Settings::Init
===================
*/
bool Settings::Init()
{
    engine->Get(mesh_marks.Get());
    engine->Get(mesh_sound.Get());
    engine->Get(mesh_mine.Get());
    engine->Get(mesh_crossout.Get());

    LIB_CHECK(engine->Get(font.Get(), DATA_PACK "Font.ttf"));
    LIB_CHECK(engine->Get(tex_button.Get(), DATA_PACK "Textures/Tile.tga"));
    LIB_CHECK(engine->Get(tex_buttonPressed.Get(), DATA_PACK "Textures/TileOpen.tga"));
    LIB_CHECK(engine->Get(tex_inputField.Get(), DATA_PACK "Textures/InputField.tga"));
    LIB_CHECK(engine->Get(tex_question.Get(), DATA_PACK "Textures/Question.tga"));
    LIB_CHECK(engine->Get(tex_soundOn.Get(), DATA_PACK "Textures/SoundOn.tga"));
    LIB_CHECK(engine->Get(tex_soundOff.Get(), DATA_PACK "Textures/SoundOff.tga"));
    LIB_CHECK(engine->Get(tex_mine.Get(), DATA_PACK "Textures/Mine.tga"));

    difficulty = chosenDifficulty = libCast<Settings::difficulty_t>(game.cfg.GetInt("Difficulty", DEFAULT_DIFFICULTY));
    marksEnabled = game.cfg.GetBool("MarksEnabled", DEFAULT_MARKS_ENABLED);
    customWidth = game.cfg.GetInt("CustomWidth", DEFAULT_CUSTOM_WIDTH);
    customHeight = game.cfg.GetInt("CustomHeight", DEFAULT_CUSTOM_HEIGHT);
    customMines = game.cfg.GetInt("CustomMines", DEFAULT_CUSTOM_MINES);

    font->SetAlign(LIB_CENTER);
    font->SetSize(10);
    font->SetShadowShift(libVec2(1.0f, 1.0f));
    font->SetShadowType(libFont::SHADOW_ADDAPTIVE);

    buttonMarks.SetTexture(tex_button.Get());
    buttonMarks.SetTextScale(0.8f);

    buttonSound.SetTexture(tex_button.Get());
    buttonSound.SetTextScale(0.8f);

    for (int i = 0; i < DIFFICULTY_LEVELS; i++)
    {
        difficultyButtons[i].SetFont(font.Get());
        difficultyButtons[i].SetText(difficultyLevels[i]);
        difficultyButtons[i].SetTexture(tex_button.Get());
    }

    buttonWidth.text = customWidth;
    buttonHeight.text = customHeight;
    buttonMines.text = customMines;

    buttonWidth.SetFont(font.Get());
    buttonHeight.SetFont(font.Get());
    buttonMines.SetFont(font.Get());
    buttonMarks.SetFont(font.Get());
    buttonSound.SetFont(font.Get());

    buttonWidth.SetTexture(tex_inputField.Get());
    buttonHeight.SetTexture(tex_inputField.Get());
    buttonMines.SetTexture(tex_inputField.Get());

    buttonWidth.SetShadowType(libFont::NO_SHADOW);
    buttonHeight.SetShadowType(libFont::NO_SHADOW);
    buttonMines.SetShadowType(libFont::NO_SHADOW);

    buttonSave.SetFont(font.Get());
    buttonSave.SetTexture(tex_button.Get());
    buttonSave.SetTextScale(0.6f);
    buttonSave.SetText(L"Save");

    buttonHelp.SetFont(font.Get());
    buttonHelp.SetTexture(tex_button.Get());
    buttonHelp.SetTextScale(0.6f);
    buttonHelp.SetText(L"Help");

    difficultyButtons[chosenDifficulty].SetTexture(tex_buttonPressed.Get());

    return true;
}

/*
===================
Settings::Update
===================
*/
void Settings::Update()
{
    buttonMarks.Update();
    buttonSound.Update();
    buttonMines.Update();
    buttonWidth.Update();
    buttonHeight.Update();
    buttonSave.Update();
    buttonHelp.Update();

    if (buttonSave.IsPressed())
    {
        buttonSave.SetTexture(tex_buttonPressed.Get());
    }
    else if (buttonSave.IsReleased())
    {
        buttonSave.SetTexture(tex_button.Get());
        bool needRestart = false;

        if (buttonWidth.text.ToInt() < MINIMAL_FIELD_WIDTH)
            buttonWidth.text = MINIMAL_FIELD_WIDTH;

        if (buttonHeight.text.ToInt() < MINIMAL_FIELD_HEIGHT)
            buttonHeight.text = MINIMAL_FIELD_HEIGHT;

        if (buttonMines.text.ToInt() < MINIMAL_MINES)
            buttonMines.text = MINIMAL_MINES;

        if (buttonMines.text.ToInt() >= buttonWidth.text.ToInt() * buttonHeight.text.ToInt())
            buttonMines.text = buttonWidth.text.ToInt() * buttonHeight.text.ToInt() - 1;

        if (difficulty != chosenDifficulty)
            needRestart = true;

        if (buttonWidth.text.ToInt() != customWidth)
            needRestart = true;

        if (buttonHeight.text.ToInt() != customHeight)
            needRestart = true;

        if (buttonMines.text.ToInt() != customMines)
            needRestart = true;

        if (needRestart)
        {
            difficulty = chosenDifficulty;
            customWidth = buttonWidth.text.ToInt();
            customHeight = buttonHeight.text.ToInt();
            customMines = buttonMines.text.ToInt();

            game.SaveSettings();
            game.Restart();
        }

        game.cfg.SetInt("Difficulty", difficulty);
        game.cfg.SetInt("CustomWidth", buttonWidth.text.ToInt());
        game.cfg.SetInt("CustomHeight", buttonHeight.text.ToInt());
        game.cfg.SetInt("CustomMines", buttonMines.text.ToInt());
        game.cfg.SetBool("MarksEnabled", marksEnabled);

        game.ToggleSettings();
        return;
    }

    if (buttonHelp.IsPressed())
    {
        buttonHelp.SetTexture(tex_buttonPressed.Get());
    }
    else if (buttonHelp.IsReleased())
    {
        buttonHelp.SetTexture(tex_button.Get());
        game.ShowHelp();
    }

    for (int i = 0; i < DIFFICULTY_LEVELS; i++)
    {
        difficultyButtons[i].Update();

        if (difficultyButtons[i].IsPressed())
        {
            for (int j = 0; j < DIFFICULTY_LEVELS; j++)
                difficultyButtons[j].SetTexture(tex_button.Get());

            difficultyButtons[i].SetTexture(tex_buttonPressed.Get());
        }
    }

    if (difficultyButtons[BEGINNER].IsReleased())
    {
        chosenDifficulty = BEGINNER;
    }
    else if (difficultyButtons[INTERMEDIATE].IsReleased())
    {
        chosenDifficulty = INTERMEDIATE;
    }
    else if (difficultyButtons[EXPERT].IsReleased())
    {
        chosenDifficulty = EXPERT;
    }
    else if (difficultyButtons[AUTO].IsReleased())
    {
        chosenDifficulty = AUTO;
    }
    else if (difficultyButtons[CUSTOM].IsReleased())
    {
        chosenDifficulty = CUSTOM;

        buttonWidth.SetColor({ libColor(1.0f, 1.0f, 1.0f), libColor(1.0f, 1.0f, 1.0f), libColor(1.0f, 1.0f, 1.0f) });
        buttonHeight.SetColor({ libColor(1.0f, 1.0f, 1.0f), libColor(1.0f, 1.0f, 1.0f), libColor(1.0f, 1.0f, 1.0f) });
        buttonMines.SetColor({ libColor(1.0f, 1.0f, 1.0f), libColor(1.0f, 1.0f, 1.0f), libColor(1.0f, 1.0f, 1.0f) });
    }

    if (buttonMarks.IsPressed())
        buttonMarks.SetTexture(tex_buttonPressed.Get());
    else if (buttonMarks.IsReleased())
        marksEnabled = !marksEnabled;
    else
        buttonMarks.SetTexture(tex_button.Get());

    if (buttonSound.IsPressed())
        buttonSound.SetTexture(tex_buttonPressed.Get());
    else if (buttonSound.IsReleased())
        game.ToggleAudio();
    else
        buttonSound.SetTexture(tex_button.Get());

    if (engine->IsKeyPressed(LIBK_MOUSE_LEFT))
    {
        if (!buttonWidth.IsPressed() && !buttonHeight.IsPressed() && !buttonMines.IsPressed())
        {
            selectedButton = nullptr;
            buttonWidth.SetColor({ LIB_COLOR_WHITE, LIB_COLOR_WHITE, LIB_COLOR_WHITE });
            buttonHeight.SetColor({ LIB_COLOR_WHITE, LIB_COLOR_WHITE, LIB_COLOR_WHITE });
            buttonMines.SetColor({ LIB_COLOR_WHITE, LIB_COLOR_WHITE, LIB_COLOR_WHITE });
        }

        if ((selectedButton != &buttonWidth && buttonWidth.IsPressed()) || 
            (selectedButton != &buttonHeight && buttonHeight.IsPressed()) ||
            (selectedButton != &buttonMines && buttonMines.IsPressed()))
        {
            selectedButton = nullptr;
            buttonWidth.SetColor({ LIB_COLOR_WHITE, LIB_COLOR_WHITE, LIB_COLOR_WHITE });
            buttonHeight.SetColor({ LIB_COLOR_WHITE, LIB_COLOR_WHITE, LIB_COLOR_WHITE });
            buttonMines.SetColor({ LIB_COLOR_WHITE, LIB_COLOR_WHITE, LIB_COLOR_WHITE });
        }
    }

    if (chosenDifficulty != CUSTOM)
    {
        buttonWidth.SetColor({ libColor(0.8f, 0.8f, 0.8f), libColor(0.8f, 0.8f, 0.8f), libColor(0.8f, 0.8f, 0.8f) });
        buttonHeight.SetColor({ libColor(0.8f, 0.8f, 0.8f), libColor(0.8f, 0.8f, 0.8f), libColor(0.8f, 0.8f, 0.8f) });
        buttonMines.SetColor({ libColor(0.8f, 0.8f, 0.8f), libColor(0.8f, 0.8f, 0.8f), libColor(0.8f, 0.8f, 0.8f) });
    }
    else if (!selectedButton)
    {
        if (buttonWidth.IsPressed())
        {
            selectedButton = &buttonWidth;
            buttonWidth.SetColor({ libColor(0.9f, 0.9f, 0.9f), libColor(0.9f, 0.9f, 0.9f), libColor(0.9f, 0.9f, 0.9f) });
        }

        if (buttonHeight.IsPressed())
        {
            selectedButton = &buttonHeight;
            buttonHeight.SetColor({ libColor(0.9f, 0.9f, 0.9f), libColor(0.9f, 0.9f, 0.9f), libColor(0.9f, 0.9f, 0.9f) });
        }

        if (buttonMines.IsPressed())
        {
            selectedButton = &buttonMines;
            buttonMines.SetColor({ libColor(0.9f, 0.9f, 0.9f), libColor(0.9f, 0.9f, 0.9f), libColor(0.9f, 0.9f, 0.9f) });
        }
    }

    if (selectedButton)
    {
        int key = engine->CurrentKey();

        if (key)
        {
            int maxValue = 0;
            int maxDigits = 3;

            if (selectedButton == &buttonWidth)
            {
                maxValue = MAXIMAL_FIELD_WIDTH;
            }
            else if (selectedButton == &buttonHeight)
            {
                maxValue = MAXIMAL_FIELD_HEIGHT;
            }
            else if (selectedButton == &buttonMines)
            {
                maxValue = buttonWidth.text.ToInt() * buttonHeight.text.ToInt() - 1;
                maxDigits = 4;
            }

            if (selectedButton->text.Length() < maxDigits)
            {
                char val = engine->KeyValue(key);

                if (libStr::IsNumeric(val))
                {
                    int digit = libStr::ToInt(&val);
                    selectedButton->text.Append(digit);

                    if (selectedButton->text.ToInt() > maxValue)
                        selectedButton->text = maxValue;
                }
            }

            if (engine->IsKeyPressed(LIBK_BACKSPACE))
                selectedButton->text.Erase(selectedButton->text.Length() - 1);
        }
    }
}

/*
===================
Settings::Draw
===================
*/
void Settings::Draw()
{
    libVec2i screenSize(engine->State(LIB_SCREEN_WIDTH), engine->State(LIB_SCREEN_HEIGHT));

    float x = screenSize.x / 2.0f;
    float y = 0.0f;
    float width = font->Size() * 13.0f;
    float height = screenSize.y * 0.09f;
    float halfTile = TILE_SIZE / 2.0f;
    float size = screenSize.x * 0.1f;
    float iconUnpressed = size / 2.0f * 0.7f;
    float iconPressed = iconUnpressed * 0.9f;
    libQuad q_icon(libVertex(-iconUnpressed, -iconUnpressed, 0.0f, 0.0f), libVertex(iconUnpressed, iconUnpressed, 1.0f, 1.0f));
    libQuad q_iconPressed(libVertex(-iconPressed, -iconPressed, 0.0f, 0.0f), libVertex(iconPressed, iconPressed, 1.0f, 1.0f));
    libVec2 marksPos(MARGIN_X * 2.0f + size / 2.0f, MARGIN_Y * 2.0f + size / 2.0f);
    libVec2 soundPos(libCast<float>(screenSize.x) - (MARGIN_X * 2.0f + size / 2.0f), MARGIN_Y * 2.0f + size / 2.0f);

    font->SetSize(screenSize.y / 100 * 5);

    buttonMarks.SetSize(size, size);
    buttonMarks.SetPosition(marksPos.x, marksPos.y);
    buttonSound.SetSize(size, size);
    buttonSound.SetPosition(soundPos.x, soundPos.y);

    buttonMarks.Draw();
    buttonSound.Draw();

    // Marks (?)
    mesh_marks->Clear();
    mesh_crossout->Clear();
    mesh_marks->Add(buttonMarks.IsPressed() ? q_iconPressed : q_icon, libVec3(marksPos.x, marksPos.y, 0.0f));

    if (!marksEnabled)
    {
        float crossoutSize = size / 2.0f * 0.7f;

        engine->DrawLine(mesh_crossout.Get(), libVertex(marksPos.x - crossoutSize, marksPos.y - crossoutSize, LIB_COLOR_RED),
                           libVertex(marksPos.x + crossoutSize, marksPos.y + crossoutSize, LIB_COLOR_RED), 1.5f);

        engine->DrawLine(mesh_crossout.Get(), libVertex(marksPos.x + crossoutSize, marksPos.y - crossoutSize, LIB_COLOR_RED),
                         libVertex(marksPos.x - crossoutSize, marksPos.y + crossoutSize, LIB_COLOR_RED), 1.5f);
    }

    // Sound on/off
    mesh_sound->Clear();
    mesh_sound->Add(buttonSound.IsPressed() ? q_iconPressed : q_icon, libVec3(soundPos.x, soundPos.y, 0.0f));

    // Difficulty section
    font->SetColor(LIB_COLOR_WHITE);
    font->SetSize(screenSize.y / 100 * 5);
    y = libCast<float>(font->LineHeight());
    font->Print2D(x, y, "Difficulty");

    y += libCast<float>(font->LineHeight()) * 1.5f;

    for (int i = 0; i < DIFFICULTY_LEVELS; i++)
    {
        difficultyButtons[i].SetTextScale(0.5f);
        difficultyButtons[i].SetSize(width, height);
        difficultyButtons[i].SetPosition(x, y);
        difficultyButtons[i].Draw();
        y += height * 1.1f;
    }

    y += height * 0.2f;

    int fontSize = libCast<int>(height / 3.0f);
    font->SetSize(fontSize);
    font->SetColor(LIB_COLOR_BLACK);
    buttonWidth.SetText(libWFormat(L"%d", buttonWidth.text.ToInt()));
    buttonWidth.SetSize(font->Size() * 5.0f, height);
    buttonWidth.SetTextScale(0.7f);
    buttonWidth.SetTextColor({ LIB_COLOR_BLACK, LIB_COLOR_BLACK, LIB_COLOR_BLACK });
    buttonWidth.SetPosition(screenSize.x / 2.0f - font->Size() * 5.0f, y);
    buttonHeight.SetText(libWFormat(L"%d", buttonHeight.text.ToInt()));
    buttonHeight.SetSize(font->Size() * 5.0f, height);
    buttonHeight.SetTextScale(0.7f);
    buttonHeight.SetTextColor({ LIB_COLOR_BLACK, LIB_COLOR_BLACK, LIB_COLOR_BLACK });
    buttonHeight.SetPosition(screenSize.x / 2.0f + font->Size() * 5.0f, y);

    auto t = font->ShadowType();
    font->SetShadowType(libFont::NO_SHADOW);
    font->SetSize(fontSize);
    font->SetColor(LIB_COLOR_BLACK);
    font->Print2D(screenSize.x / 2.0f, y, "X");

    y += height * 1.2f;

    libQuad q_tile(libVertex(-halfTile, -halfTile, 0.0f, 0.0f), libVertex(halfTile, halfTile, 1.0f, 1.0f));
    mesh_mine->Clear();
    mesh_mine->Add(q_tile, libVec3(screenSize.x / 2.0f - font->Size() * 6.0f, y, 0.0f));
    font->SetSize(fontSize);
    font->Print2D(screenSize.x / 2.0f - font->Size() * 3.0f, y, "=");
    font->SetColor(LIB_COLOR_WHITE);

    buttonMines.SetText(libWFormat(L"%d", buttonMines.text.ToInt()));
    buttonMines.SetSize(font->Size() * 10.0f, height);
    buttonMines.SetTextScale(0.7f);
    buttonMines.SetTextColor({ LIB_COLOR_BLACK, LIB_COLOR_BLACK, LIB_COLOR_BLACK });
    buttonMines.SetPosition(screenSize.x / 2.0f + buttonMines.size.x * 0.35f, y);
    font->SetShadowType(t);
    
    buttonWidth.Draw();
    buttonHeight.Draw();
    buttonMines.Draw();

    y = screenSize.y - MARGIN_Y * 2 - buttonSave.size.y / 2.0f;

    buttonSave.SetSize(screenSize.x * 0.4f, height);
    buttonSave.SetPosition(MARGIN_X * 2 + buttonSave.size.x / 2.0f, y);
    buttonSave.Draw();

    buttonHelp.SetSize(screenSize.x * 0.4f, height);
    buttonHelp.SetPosition(screenSize.x - MARGIN_X * 2 - buttonSave.size.x / 2.0f, y);
    buttonHelp.Draw();

    if (engine->State(LIB_AUDIO_VOLUME))
        engine->Draw(mesh_sound.Get(), tex_soundOn.Get(), true);
    else
        engine->Draw(mesh_sound.Get(), tex_soundOff.Get(), true);

    engine->Draw(mesh_marks.Get(), tex_question.Get(), true);
    engine->Draw(mesh_mine.Get(), tex_mine.Get(), true);
    engine->Draw(mesh_crossout.Get(), nullptr, true);
}

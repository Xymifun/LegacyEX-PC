#include "platform/pc/PcTextKeyboard.hpp"

#include "platform/pc/PcChat.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/Font.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Gui.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/System.hpp"
#include "game/util/ResourceLocation.hpp"
#include "platform/pc/PcOs.hpp"

#include <string>
#include <vpad/input.h>

namespace
{
enum SpecialKey
{
    eKey_Shift,
    eKey_Symbols,
    eKey_Space,
    eKey_Delete,
    eKey_Done,
    eKey_Count
};

enum Direction
{
    eDirection_None,
    eDirection_Up,
    eDirection_Down,
    eDirection_Left,
    eDirection_Right
};

const int kColumns = 10;
const int kCharacterRows = 4;
const int kRows = kCharacterRows + 1;
const char* const kLetterRows[kCharacterRows] = {"1234567890", "qwertyuiop", "asdfghjkl:", "zxcvbnm,.-"};
const char* const kSymbolRows[kCharacterRows] = {"1234567890", "!@#$%^&*()", "-_=+[]{};:", "'\"<>/?\\|~`"};
const wchar_t* const kSpecialLabels[eKey_Count] = {L"Shift", L"!#1", L"Space", L"Delete", L"Done"};
const unsigned long long kFirstRepeatNanoseconds = 400000000ULL;
const unsigned long long kRepeatNanoseconds = 90000000ULL;
const float kStickThreshold = 0.5f;

const int kScale = 3;
const int kVirtualWidth = 1280 / kScale;
const int kVirtualHeight = 720 / kScale;
const int kKeyWidth = 22;
const int kKeyHeight = 20;
const int kKeyPitchX = 24;
const int kKeyPitchY = 22;
const int kKeyboardWidth = kColumns * kKeyPitchX - (kKeyPitchX - kKeyWidth);
const int kKeyboardLeft = (kVirtualWidth - kKeyboardWidth) / 2;
const int kKeyboardTop = 64;
const int kButtonTextureWidth = 200;
const int kButtonNormalV = 66;
const int kButtonSelectedV = 86;

bool s_shown = false;
bool s_shift = false;
bool s_symbols = false;
int s_row = 1;
int s_column = 0;
Direction s_heldDirection = eDirection_None;
unsigned long long s_nextRepeat = 0;
unsigned int s_keyboardTyping = 0;
ResourceLocation s_buttonTexture(TN_GUI_GUI);

wchar_t CharacterAt(int row, int column)
{
    const char* const* rows = s_symbols ? kSymbolRows : kLetterRows;
    wchar_t character = static_cast<wchar_t>(rows[row][column]);
    if (s_shift && character >= L'a' && character <= L'z')
        character = static_cast<wchar_t>(character - L'a' + L'A');
    return character;
}

void PlayPress()
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
}

void PlayFocus()
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
}

void Move(Direction direction)
{
    switch (direction)
    {
    case eDirection_Up:
        s_row = (s_row + kRows - 1) % kRows;
        break;
    case eDirection_Down:
        s_row = (s_row + 1) % kRows;
        break;
    case eDirection_Left:
        if (s_row == kCharacterRows)
            s_column = ((s_column / 2) * 2 + kColumns - 2) % kColumns;
        else
            s_column = (s_column + kColumns - 1) % kColumns;
        break;
    case eDirection_Right:
        if (s_row == kCharacterRows)
            s_column = ((s_column / 2) * 2 + 2) % kColumns;
        else
            s_column = (s_column + 1) % kColumns;
        break;
    default:
        return;
    }
    PlayFocus();
}

Direction HeldDirection(uint32_t hold, float stickX, float stickY)
{
    if ((hold & VPAD_BUTTON_UP) != 0 || stickY > kStickThreshold)
        return eDirection_Up;
    if ((hold & VPAD_BUTTON_DOWN) != 0 || stickY < -kStickThreshold)
        return eDirection_Down;
    if ((hold & VPAD_BUTTON_LEFT) != 0 || stickX < -kStickThreshold)
        return eDirection_Left;
    if ((hold & VPAD_BUTTON_RIGHT) != 0 || stickX > kStickThreshold)
        return eDirection_Right;
    return eDirection_None;
}

void UpdateDirection(uint32_t hold, float stickX, float stickY)
{
    const Direction direction = HeldDirection(hold, stickX, stickY);
    const unsigned long long now = PcOsMonotonicNanoseconds();
    if (direction != s_heldDirection)
    {
        s_heldDirection = direction;
        s_nextRepeat = now + kFirstRepeatNanoseconds;
        Move(direction);
    }
    else if (direction != eDirection_None && now >= s_nextRepeat)
    {
        s_nextRepeat = now + kRepeatNanoseconds;
        Move(direction);
    }
}

void Delete()
{
    PcTextEntryBackspace();
    PlayPress();
}

void ActivateSelected()
{
    PlayPress();
    if (s_row < kCharacterRows)
    {
        PcTextEntryType(static_cast<char16_t>(CharacterAt(s_row, s_column)));
        return;
    }
    switch (s_column / 2)
    {
    case eKey_Shift:
        s_shift = !s_shift;
        break;
    case eKey_Symbols:
        s_symbols = !s_symbols;
        break;
    case eKey_Space:
        PcTextEntryType(u' ');
        break;
    case eKey_Delete:
        PcTextEntryBackspace();
        break;
    case eKey_Done:
        PcTextEntrySetResult(ePcTextEntry_Accepted);
        break;
    }
}

struct ButtonNames
{
    const wchar_t* type;
    const wchar_t* erase;
    const wchar_t* space;
    const wchar_t* shift;
    const wchar_t* symbols;
    const wchar_t* done;
};

ButtonNames NamesForGlyphs()
{
    switch (PcInputGlyphSet())
    {
    case ePcGlyphSet_PS3:
    {
        const ButtonNames names = {L"Cross", L"Circle", L"Square", L"Triangle", L"L1", L"Start"};
        return names;
    }
    case ePcGlyphSet_PS4:
    {
        const ButtonNames names = {L"Cross", L"Circle", L"Square", L"Triangle", L"L1", L"Options"};
        return names;
    }
    case ePcGlyphSet_Xbox360:
    {
        const ButtonNames names = {L"A", L"B", L"X", L"Y", L"LB", L"Start"};
        return names;
    }
    case ePcGlyphSet_WiiU:
    {
        const ButtonNames names = {L"A", L"B", L"Y", L"X", L"L", L"+"};
        return names;
    }
    default:
    {
        const ButtonNames names = {L"A", L"B", L"X", L"Y", L"LB", L"Menu"};
        return names;
    }
    }
}

void DrawTexturedQuad(float x, float y, float width, float height, float u, float v)
{
    const float scale = 1.0f / 256.0f;
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->begin();
    builder->vertexUV(x, y + height, 0.0f, u * scale, (v + height) * scale);
    builder->vertexUV(x + width, y + height, 0.0f, (u + width) * scale, (v + height) * scale);
    builder->vertexUV(x + width, y, 0.0f, (u + width) * scale, v * scale);
    builder->vertexUV(x, y, 0.0f, u * scale, v * scale);
    builder->end();
}

void DrawButton(int x, int y, int width, bool selected)
{
    const int v = selected ? kButtonSelectedV : kButtonNormalV;
    const int left = width / 2;
    const int right = width - left;
    DrawTexturedQuad(static_cast<float>(x), static_cast<float>(y), static_cast<float>(left),
                     static_cast<float>(kKeyHeight), 0.0f, static_cast<float>(v));
    DrawTexturedQuad(static_cast<float>(x + left), static_cast<float>(y), static_cast<float>(right),
                     static_cast<float>(kKeyHeight), static_cast<float>(kButtonTextureWidth - right),
                     static_cast<float>(v));
}

void KeyRect(int row, int index, int* x, int* y, int* width)
{
    *y = kKeyboardTop + row * kKeyPitchY;
    if (row < kCharacterRows)
    {
        *x = kKeyboardLeft + index * kKeyPitchX;
        *width = kKeyWidth;
    }
    else
    {
        *x = kKeyboardLeft + index * 2 * kKeyPitchX;
        *width = kKeyPitchX + kKeyWidth;
    }
}

void DrawKeyboard(Minecraft* minecraft, Font* font)
{
    const int selectedIndex = s_row < kCharacterRows ? s_column : s_column / 2;
    minecraft->mTextures->clearLastBoundId();
    minecraft->mTextures->bindTexture(&s_buttonTexture);
    GlStateManager::enableTexture();
    GlStateManager::disableBlend();
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    for (int row = 0; row < kRows; ++row)
    {
        const int count = row < kCharacterRows ? kColumns : eKey_Count;
        for (int index = 0; index < count; ++index)
        {
            int x, y, width;
            KeyRect(row, index, &x, &y, &width);
            DrawButton(x, y, width, row == s_row && index == selectedIndex);
        }
    }
    minecraft->mTextures->clearLastBoundId();
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);
    for (int row = 0; row < kRows; ++row)
    {
        const int count = row < kCharacterRows ? kColumns : eKey_Count;
        for (int index = 0; index < count; ++index)
        {
            int x, y, width;
            KeyRect(row, index, &x, &y, &width);
            std::wstring label;
            if (row < kCharacterRows)
                label.assign(1, CharacterAt(row, index));
            else if (index == eKey_Symbols && s_symbols)
                label = L"ABC";
            else
                label = kSpecialLabels[index];
            const bool selected = row == s_row && index == selectedIndex;
            const bool toggled = row == kCharacterRows
                                 && ((index == eKey_Shift && s_shift) || (index == eKey_Symbols && s_symbols));
            int colour = static_cast<int>(selected ? 0xFFFFFFA0u : 0xFFE0E0E0u);
            if (toggled)
                colour = static_cast<int>(0xFF55FF55u);
            font->drawShadow(label, x + (width - font->width(label) + 1) / 2, y + (kKeyHeight - 8) / 2, colour);
        }
    }
}
}

void PcTextKeyboardBegin()
{
    const PcGlyphSet glyphs = PcInputGlyphSet();
    s_shown = glyphs != ePcGlyphSet_Keyboard;
    s_shift = false;
    s_symbols = false;
    s_row = 1;
    s_column = 0;
    s_heldDirection = eDirection_None;
    s_keyboardTyping = PcTextEntryKeyboardTyping();
}

PcTextEntryResult PcTextKeyboardUpdate(uint32_t hold, uint32_t trigger, float stickX, float stickY)
{
    if (!PcTextEntryActive())
        return ePcTextEntry_Pending;
    if (PcTextEntryKeyboardTyping() != s_keyboardTyping)
    {
        s_keyboardTyping = PcTextEntryKeyboardTyping();
        s_shown = false;
    }
    if (PcTextEntryCurrentResult() != ePcTextEntry_Pending)
        return PcTextEntryCurrentResult();
    if (!s_shown)
    {
        if (trigger != 0 || HeldDirection(0, stickX, stickY) != eDirection_None)
        {
            s_shown = true;
            s_heldDirection = HeldDirection(hold, stickX, stickY);
            s_nextRepeat = PcOsMonotonicNanoseconds() + kFirstRepeatNanoseconds;
        }
        return ePcTextEntry_Pending;
    }
    UpdateDirection(hold, stickX, stickY);
    if ((trigger & VPAD_BUTTON_A) != 0)
        ActivateSelected();
    if ((trigger & VPAD_BUTTON_B) != 0)
    {
        if (PcTextEntryText()[0] != 0)
            Delete();
        else
        {
            gConsoleUIController.PlayUISFX(SoundEvents::UI_BACK);
            PcTextEntrySetResult(ePcTextEntry_Cancelled);
        }
    }
    if ((trigger & VPAD_BUTTON_Y) != 0)
    {
        PcTextEntryType(u' ');
        PlayPress();
    }
    if ((trigger & (VPAD_BUTTON_X | VPAD_BUTTON_R | VPAD_BUTTON_ZR)) != 0)
    {
        s_shift = !s_shift;
        PlayPress();
    }
    if ((trigger & (VPAD_BUTTON_L | VPAD_BUTTON_ZL)) != 0)
    {
        s_symbols = !s_symbols;
        PlayPress();
    }
    if ((trigger & VPAD_BUTTON_PLUS) != 0)
    {
        PlayPress();
        PcTextEntrySetResult(ePcTextEntry_Accepted);
    }
    return PcTextEntryCurrentResult();
}

void PcTextKeyboardDraw()
{
    if (!PcTextEntryActive() || PcChatActive())
        return;
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft == NULL || minecraft->mFont == NULL || minecraft->mGui == NULL)
        return;
    Font* font = minecraft->mFont;
    const int boxLeft = s_shown ? kKeyboardLeft - 20 : 30;
    const int boxRight = s_shown ? kKeyboardLeft + kKeyboardWidth + 20 : kVirtualWidth - 30;
    const int boxTop = s_shown ? 32 : kVirtualHeight / 2 - 12;
    const int boxBottom = boxTop + 22;

    gRenderer->StartFrame();
    GlStateManager::matrixMode(1);
    GlStateManager::loadIdentity();
    GlStateManager::ortho(0.0, 1280.0, 720.0, 0.0, 1000.0, 3000.0);
    GlStateManager::matrixMode(0);
    GlStateManager::loadIdentity();
    GlStateManager::translatef(0.0f, 0.0f, -2000.0f);
    GlStateManager::scalef(static_cast<float>(kScale), static_cast<float>(kScale), 1.0f);
    GlStateManager::disableDepthTest();
    GlStateManager::disableAlphaTest();
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);
    minecraft->mGui->fill(0, 0, kVirtualWidth + 1, kVirtualHeight + 1,
                          static_cast<int>(s_shown ? 0xC0000000u : 0xA0000000u));
    minecraft->mGui->fill(boxLeft - 1, boxTop - 1, boxRight + 1, boxBottom + 1, static_cast<int>(0xFFA0A0A0u));
    minecraft->mGui->fill(boxLeft, boxTop, boxRight, boxBottom, static_cast<int>(0xFF000000u));
    if (s_shown)
        DrawKeyboard(minecraft, font);
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    minecraft->mTextures->clearLastBoundId();
    PcTextEntryDrawLine(font, minecraft->mGui, boxLeft + 5, boxTop + 7, boxRight - boxLeft - 10,
                        static_cast<int>(0xFFFFFFFFu), (System::processTimeInMilliSecs() / 500) % 2 == 0);
    std::wstring hint(L"Enter: Done    Esc: Cancel");
    std::wstring secondHint;
    int hintY = boxBottom + 8;
    if (s_shown)
    {
        const ButtonNames names = NamesForGlyphs();
        hint = std::wstring(names.type) + L" Type    " + names.erase + L" Delete    " + names.space + L" Space";
        secondHint = std::wstring(names.shift) + L" Shift    " + names.symbols + L" Symbols    " + names.done + L" Done";
        if (font->width(hint + L"    " + secondHint) <= kVirtualWidth - 12)
        {
            hint += L"    " + secondHint;
            secondHint.clear();
        }
        hintY = kKeyboardTop + kRows * kKeyPitchY + 4;
    }
    font->drawShadow(hint, (kVirtualWidth - font->width(hint)) / 2, hintY, static_cast<int>(0xFFE0E0E0u));
    if (!secondHint.empty())
        font->drawShadow(secondHint, (kVirtualWidth - font->width(secondHint)) / 2, hintY + 10,
                         static_cast<int>(0xFFE0E0E0u));
    minecraft->mTextures->clearLastBoundId();
    GlStateManager::enableAlphaTest();
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void PcTextEntryDrawLine(Font* font, Gui* gui, int x, int y, int width, int color, bool caretOn)
{
    std::wstring text;
    for (const char16_t* character = PcTextEntryText(); *character != 0; ++character)
        text.push_back(static_cast<wchar_t>(*character));
    std::wstring::size_type cursor = PcTextEntryCursor();
    if (cursor > text.length())
        cursor = text.length();
    std::wstring::size_type start = 0;
    while (start < cursor && font->width(text.substr(start, cursor - start) + L"_") > width)
        ++start;
    std::wstring visible = text.substr(start);
    while (!visible.empty() && font->width(visible) > width)
        visible.erase(visible.length() - 1);
    font->drawShadow(visible, x, y, color);
    if (!caretOn)
        return;
    const int caretX = x + font->width(text.substr(start, cursor - start));
    if (cursor == text.length())
        font->drawShadow(L"_", caretX, y, color);
    else
    {
        gui->fill(caretX, y - 1, caretX + 1, y + 9, static_cast<int>(0xFFD0D0D0u));
        GlStateManager::enableBlend();
        GlStateManager::blendFunc(4, 5);
        GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
        Minecraft::GetInstance()->mTextures->clearLastBoundId();
    }
}

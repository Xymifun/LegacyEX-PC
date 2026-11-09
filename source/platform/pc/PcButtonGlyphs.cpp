#include "platform/pc/PcButtonGlyphs.hpp"

#include "platform/pc/PcInput.hpp"
#include "platform/pc/PcKeyboardGlyphs.hpp"
#include "platform/pc/PcPaths.hpp"

#include "game/client/render/Renderer.hpp"

#include <map>
#include <vpad/input.h>
#include <png.h>
#include <string.h>
#include <string>
#include <vector>

namespace
{
enum Glyph
{
    eGlyph_A,
    eGlyph_B,
    eGlyph_X,
    eGlyph_Y,
    eGlyph_L,
    eGlyph_R,
    eGlyph_ZL,
    eGlyph_ZR,
    eGlyph_Plus,
    eGlyph_Minus,
    eGlyph_LeftStickPress,
    eGlyph_RightStickPress,
    eGlyph_LeftStick,
    eGlyph_LeftStickNavigate,
    eGlyph_LeftStickSides,
    eGlyph_RightStick,
    eGlyph_Up,
    eGlyph_Down,
    eGlyph_Left,
    eGlyph_Right,
    eGlyph_None
};

enum Context
{
    eContext_Game,
    eContext_Menu,
    eContext_Container
};

struct GlyphName
{
    const char* prefix;
    Glyph glyph;
};

const GlyphName kGlyphNames[] = {
    {"ActionRight", eGlyph_A},
    {"ActionBottom", eGlyph_B},
    {"ActionTop", eGlyph_X},
    {"ActionLeft", eGlyph_Y},
    {"ShoulderL", eGlyph_L},
    {"ShoulderR", eGlyph_R},
    {"TriggerL", eGlyph_ZL},
    {"TriggerR", eGlyph_ZR},
    {"SystemStart", eGlyph_Plus},
    {"SystemSelect", eGlyph_Minus},
    {"LeftStickButton", eGlyph_LeftStickPress},
    {"RightStickButton", eGlyph_RightStickPress},
    {"LeftStickNavigate", eGlyph_LeftStickNavigate},
    {"LeftStickSides", eGlyph_LeftStickSides},
    {"LeftStick", eGlyph_LeftStick},
    {"RightStickNavigate", eGlyph_RightStick},
    {"RightStick", eGlyph_RightStick},
    {"DPadUp", eGlyph_Up},
    {"DPadDown", eGlyph_Down},
    {"DPadLeft", eGlyph_Left},
    {"DPadRight", eGlyph_Right},
};

const char* const kPadFiles[eGlyph_None] = {
    "ButtonA",          "ButtonB",          "ButtonY",          "ButtonX",
    "ButtonLeftBumper", "ButtonRightBumper", "ButtonLeftTrigger", "ButtonRightTrigger",
    "ButtonStart",      "ButtonBack",       "ButtonLS",         "ButtonRS",
    "ButtonLeftStick",  "ButtonLeftStick_Navigate", "ButtonLeftStick_sides", "ButtonRightStick",
    "ButtonDpadU",      "ButtonDpadD",      "ButtonDpadL",      "ButtonDpadR",
};

const char* const kKeyboardFiles[3][eGlyph_None] = {
    {"KeySpace", "KeyQ", "KeyE", "KeyC", "MouseWheelUp", "MouseWheelDown", "MouseRight", "MouseLeft", "KeyEscape",
     "KeyTab", "KeyF5", "KeyShift", "KeyWASD", "KeyWASD", "KeyAD", "Mouse", "KeyUp", "KeyDown", "KeyLeft",
     "KeyRight"},
    {"KeyEnter", "KeyEscape", "KeyE", "KeyC", "KeyZ", "KeyX", "KeyR", "KeyF", "KeyEscape", "KeyTab", 0, 0, "KeyWASD",
     "KeyWASD", "KeyAD", "MouseWheelDown", "KeyUp", "KeyDown", "KeyLeft", "KeyRight"},
    {"MouseLeft", "KeyEscape", "KeyShift", "MouseRight", "KeyZ", "KeyX", "KeyR", "KeyF", "KeyEscape", "KeyTab", 0, 0,
     "KeyWASD", "KeyWASD", "KeyAD", "Mouse", "KeyUp", "KeyDown", "KeyLeft", "KeyRight"},
};

const uint32_t kGlyphButtons[eGlyph_None] = {
    VPAD_BUTTON_A,       VPAD_BUTTON_B,       VPAD_BUTTON_X,  VPAD_BUTTON_Y,  VPAD_BUTTON_L,    VPAD_BUTTON_R,
    VPAD_BUTTON_ZL,      VPAD_BUTTON_ZR,      VPAD_BUTTON_PLUS, VPAD_BUTTON_MINUS, VPAD_BUTTON_STICK_L,
    VPAD_BUTTON_STICK_R, 0,                   0,              0,              0,                VPAD_BUTTON_UP,
    VPAD_BUTTON_DOWN,    VPAD_BUTTON_LEFT,    VPAD_BUTTON_RIGHT};

const PcKeyAction kPromptPriority[] = {
    ePcKey_Jump,         ePcKey_Action,       ePcKey_Use,          ePcKey_Drop,         ePcKey_Inventory,
    ePcKey_Crafting,     ePcKey_Sneak,        ePcKey_ChangeCamera, ePcKey_CycleLeft,    ePcKey_CycleRight,
    ePcKey_PickBlock,    ePcKey_FlyUp,        ePcKey_FlyDown,      ePcKey_FlyLeft,      ePcKey_FlyRight,
    ePcKey_ConfirmReady, ePcKey_Vote,         ePcKey_SpectatePrevious, ePcKey_SpectateNext, ePcKey_SpectateNoise,
    ePcKey_SpectateCancel, ePcKey_RestartSection, ePcKey_RestartRace, ePcKey_LookBehind};

const char* GameKeyboardFile(Glyph glyph)
{
    switch (glyph)
    {
    case eGlyph_Plus:
        return "KeyEscape";
    case eGlyph_Minus:
        return PcKeyGlyphName(PcKeyBinding(ePcKey_PlayerList));
    case eGlyph_LeftStick:
    case eGlyph_LeftStickNavigate:
        return PcKeyBindingsDefaultMovement() ? "KeyWASD" : PcKeyGlyphName(PcKeyBinding(ePcKey_Forward));
    case eGlyph_LeftStickSides:
        return PcKeyBindingsDefaultMovement() ? "KeyAD" : PcKeyGlyphName(PcKeyBinding(ePcKey_Left));
    case eGlyph_RightStick:
        return "Mouse";
    default:
        break;
    }
    if (glyph == eGlyph_R && PcInputGlideKeys())
        return PcKeyGlyphName(PcKeyBinding(ePcKey_GlideGhost));
    const uint32_t button = kGlyphButtons[glyph];
    for (int pass = 0; pass < 2; ++pass)
    {
        for (unsigned int index = 0; index < sizeof(kPromptPriority) / sizeof(kPromptPriority[0]); ++index)
        {
            const PcKeyAction action = kPromptPriority[index];
            const PcKeySection section = PcKeyBindingSection(action);
            const bool special = section == ePcKeySection_MiniGameLobby || section == ePcKeySection_Spectating
                                 || section == ePcKeySection_Glide;
            if (special != (pass == 0) || !PcInputKeySectionActive(section))
                continue;
            const int code = PcKeyBinding(action);
            if (code != kPcKeyNone && (PcInputActionButtons(PcKeyBindingGameAction(action)) & button) != 0)
                return PcKeyGlyphName(code);
        }
    }
    if (glyph == eGlyph_L)
        return "MouseWheelUp";
    if (glyph == eGlyph_R)
        return "MouseWheelDown";
    return kKeyboardFiles[eContext_Game][glyph];
}

Glyph GlyphFromName(const char* name, bool* active)
{
    for (unsigned int index = 0; index < sizeof(kGlyphNames) / sizeof(kGlyphNames[0]); ++index)
    {
        size_t length = strlen(kGlyphNames[index].prefix);
        if (strncmp(name, kGlyphNames[index].prefix, length) != 0)
            continue;
        const char* rest = name + length;
        *active = strncmp(rest, "Active", 6) == 0;
        if (rest[0] == 0 || *active || strncmp(rest, "_Small", 6) == 0 || strncmp(rest, "_CTRL", 5) == 0)
            return kGlyphNames[index].glyph;
    }
    return eGlyph_None;
}

int UploadTexture(int width, int height, const unsigned char* rgba)
{
    int texture = gRenderer->TextureCreate();
    if (texture < 0)
        return -1;
    gRenderer->TextureBind(texture);
    gRenderer->TextureSetTextureLevels(1);
    gRenderer->TextureData(width, height, const_cast<unsigned char*>(rgba), 0, C4JRender::eTextureFormat_RGBA8);
    return texture;
}

int FinishTexture(png_image& image)
{
    image.format = PNG_FORMAT_RGBA;
    std::vector<unsigned char> pixels(PNG_IMAGE_SIZE(image));
    if (!png_image_finish_read(&image, 0, &pixels[0], 0, 0))
    {
        png_image_free(&image);
        return -1;
    }
    return UploadTexture(static_cast<int>(image.width), static_cast<int>(image.height), &pixels[0]);
}

int LoadTexture(const std::string& path)
{
    png_image image;
    memset(&image, 0, sizeof(image));
    image.version = PNG_IMAGE_VERSION;
    if (!png_image_begin_read_from_file(&image, path.c_str()))
        return -1;
    return FinishTexture(image);
}

int LoadKeyboardTexture(const std::string& name)
{
    for (unsigned int index = 0; index < kPcKeyboardGlyphCount; ++index)
    {
        if (name != kPcKeyboardGlyphs[index].name)
            continue;
        png_image image;
        memset(&image, 0, sizeof(image));
        image.version = PNG_IMAGE_VERSION;
        if (!png_image_begin_read_from_memory(&image, kPcKeyboardGlyphs[index].data, kPcKeyboardGlyphs[index].size))
            return -1;
        return FinishTexture(image);
    }
    return -1;
}

std::map<std::string, int> s_textures;

int GlyphTexture(const std::string& path, bool keyboard)
{
    std::map<std::string, int>::iterator found = s_textures.find(path);
    if (found != s_textures.end())
        return found->second;
    int texture = keyboard ? LoadKeyboardTexture(path) : LoadTexture(PcResolvePath(path.c_str()));
    s_textures[path] = texture;
    return texture;
}
}

int PcUiTexture(const std::string& path)
{
    return GlyphTexture(path, false);
}

int PcButtonGlyphTexture(const char* name, int textureId)
{
    if (strncmp(name, "PcKey_", 6) == 0)
    {
        int texture = GlyphTexture(name + 6, true);
        return texture >= 0 ? texture : textureId;
    }
    PcGlyphSet set = PcInputGlyphSet();
    if (set == ePcGlyphSet_WiiU || set == ePcGlyphSet_Auto)
        return textureId;
    bool active = false;
    Glyph glyph = GlyphFromName(name, &active);
    if (glyph == eGlyph_None)
        return textureId;
    const char* file;
    const char* folder;
    if (set == ePcGlyphSet_Keyboard)
    {
        int context = PcInputGlyphContext();
        if (context < eContext_Game || context > eContext_Container)
            context = eContext_Menu;
        file = context == eContext_Game ? GameKeyboardFile(glyph) : kKeyboardFiles[context][glyph];
        folder = "keyboard";
    }
    else
    {
        if (PcInputNintendoButtons() && (glyph == eGlyph_A || glyph == eGlyph_B))
            glyph = glyph == eGlyph_A ? eGlyph_B : eGlyph_A;
        file = kPadFiles[glyph];
        folder = set == ePcGlyphSet_PS3 ? "ps3"
                 : set == ePcGlyphSet_PS4 ? "ps4"
                 : set == ePcGlyphSet_Xbox360 ? "xbox360"
                                              : "xboxone";
    }
    if (file == 0)
        return textureId;
    const bool keyboard = set == ePcGlyphSet_Keyboard;
    std::string base = keyboard ? std::string(file) : std::string("PC/Glyphs/") + folder + "/" + file;
    const char* extension = keyboard ? "" : ".png";
    int texture = active ? GlyphTexture(base + "_use" + extension, keyboard) : -1;
    if (texture < 0)
        texture = GlyphTexture(base + extension, keyboard);
    return texture >= 0 ? texture : textureId;
}

#include "platform/pc/PcKeyBindings.hpp"

#include "platform/pc/PcPaths.hpp"

#include "game/core/GameSettings.hpp"
#include "game/core/L10N.hpp"
#include "game/core/StringIds.hpp"

#include <stdio.h>
#include <string.h>
#include <map>
#include <vector>

namespace
{
const char BINDINGS_FILE[] = "keybinds.txt";

struct BindingInfo
{
    const char* id;
    PcKeySection section;
    int gameAction;
    int label;
    const wchar_t* text;
    int code;
};

const BindingInfo kBindings[ePcKey_Count] = {
    {"forward", ePcKeySection_Movement, -1, 0, L"Walk Forwards", 'W'},
    {"back", ePcKeySection_Movement, -1, 0, L"Walk Backwards", 'S'},
    {"left", ePcKeySection_Movement, -1, 0, L"Strafe Left", 'A'},
    {"right", ePcKeySection_Movement, -1, 0, L"Strafe Right", 'D'},
    {"jump", ePcKeySection_Movement, EControllerActions_Jump, IDS_CONTROLS_JUMP, 0, ' '},
    {"sneak", ePcKeySection_Movement, EControllerActions_Sneak, IDS_CONTROLS_SNEAK, 0, 0x10},
    {"sprint", ePcKeySection_Movement, -1, IDS_CONTROLS_SPRINT, 0, 0x11},
    {"use", ePcKeySection_Standard, EControllerActions_Use, IDS_CONTROLS_USE, 0, kPcKeyMouseRight},
    {"action", ePcKeySection_Standard, EControllerActions_Action, IDS_CONTROLS_ACTION, 0, kPcKeyMouseLeft},
    {"pickblock", ePcKeySection_Standard, EControllerActions_PickBlock, IDS_CONTROLS_PICKBLOCK, 0, kPcKeyMouseMiddle},
    {"drop", ePcKeySection_Standard, EControllerActions_Drop, IDS_CONTROLS_DROP, 0, 'Q'},
    {"inventory", ePcKeySection_Standard, EControllerActions_Inventory, IDS_CONTROLS_INVENTORY, 0, 'E'},
    {"crafting", ePcKeySection_Standard, EControllerActions_Crafting, IDS_CONTROLS_CRAFTING, 0, 'C'},
    {"cycleleft", ePcKeySection_Standard, EControllerActions_CycleHeldItemLeft, IDS_CONTROLS_HELDITEM_LEFT, 0, 0},
    {"cycleright", ePcKeySection_Standard, EControllerActions_CycleHeldItemRight, IDS_CONTROLS_HELDITEM_RIGHT, 0, 0},
    {"camera", ePcKeySection_Standard, EControllerActions_ChangeCameraMode, IDS_CONTROLS_THIRDPERSON, 0, 0x74},
    {"playerlist", ePcKeySection_Standard, -1, 0, L"Player List", 0x09},
    {"chat", ePcKeySection_Standard, -1, 0, L"Open Chat", 'T'},
    {"command", ePcKeySection_Standard, -1, 0, L"Open Command", 0xBF},
    {"pushtotalk", ePcKeySection_Standard, -1, 0, L"Push to Talk", 'V'},
    {"screenshot", ePcKeySection_Standard, -1, 0, L"Take Screenshot", 0x71},
    {"hotbar1", ePcKeySection_Hotbar, -1, 0, L"Hotbar Slot 1", '1'},
    {"hotbar2", ePcKeySection_Hotbar, -1, 0, L"Hotbar Slot 2", '2'},
    {"hotbar3", ePcKeySection_Hotbar, -1, 0, L"Hotbar Slot 3", '3'},
    {"hotbar4", ePcKeySection_Hotbar, -1, 0, L"Hotbar Slot 4", '4'},
    {"hotbar5", ePcKeySection_Hotbar, -1, 0, L"Hotbar Slot 5", '5'},
    {"hotbar6", ePcKeySection_Hotbar, -1, 0, L"Hotbar Slot 6", '6'},
    {"hotbar7", ePcKeySection_Hotbar, -1, 0, L"Hotbar Slot 7", '7'},
    {"hotbar8", ePcKeySection_Hotbar, -1, 0, L"Hotbar Slot 8", '8'},
    {"hotbar9", ePcKeySection_Hotbar, -1, 0, L"Hotbar Slot 9", '9'},
    {"flyleft", ePcKeySection_Creative, EControllerActions_FlyLeft, IDS_CONTROLS_CREATIVE_FLY_LEFT, 0, 0x25},
    {"flyright", ePcKeySection_Creative, EControllerActions_FlyRight, IDS_CONTROLS_CREATIVE_FLY_RIGHT, 0, 0x27},
    {"flyup", ePcKeySection_Creative, EControllerActions_FlyUp, IDS_CONTROLS_CREATIVE_FLY_UP, 0, 0x26},
    {"flydown", ePcKeySection_Creative, EControllerActions_FlyDown, IDS_CONTROLS_CREATIVE_FLY_DOWN, 0, 0x28},
    {"ready", ePcKeySection_MiniGameLobby, EControllerActions_ConfirmReady, IDS_CONTROLS_LOBBY_READY, 0, 'C'},
    {"vote", ePcKeySection_MiniGameLobby, EControllerActions_Vote, IDS_CONTROLS_LOBBY_VOTE_SELECT, 0, 'Q'},
    {"spectateprevious", ePcKeySection_Spectating, EControllerActions_SpectatePrevious, IDS_CONTROLS_SPECTATE_PREV, 0,
     0},
    {"spectatenext", ePcKeySection_Spectating, EControllerActions_SpectateNext, IDS_CONTROLS_SPECTATE_NEXT, 0, 0},
    {"spectatenoise", ePcKeySection_Spectating, EControllerActions_SpectateNoise, IDS_CONTROLS_SPECTATE_NOISE, 0,
     kPcKeyMouseLeft},
    {"spectatecancel", ePcKeySection_Spectating, EControllerActions_SpectateCancel, IDS_CONTROLS_SPECTATE_CANCEL, 0,
     'Q'},
    {"restartsection", ePcKeySection_Glide, EControllerActions_RestartSection, IDS_TOOLTIP_RESTART_SECTION, 0, 'C'},
    {"restartrace", ePcKeySection_Glide, EControllerActions_RestartRace, IDS_TOOLTIP_RESTART_RACE, 0, 'Q'},
    {"lookbehind", ePcKeySection_Glide, EControllerActions_LookBehind, IDS_TOOLTIP_LOOK_BEHIND, 0, 'E'},
    {"glideghost", ePcKeySection_Glide, -1, 0, L"Toggle Ghost", 'G'},
};

struct KeyName
{
    int code;
    const char* glyph;
    const wchar_t* text;
};

const KeyName kKeyNames[] = {
    {0x08, "KeyBackspace", L"Backspace"},
    {0x09, "KeyTab", L"Tab"},
    {0x0D, "KeyEnter", L"Enter"},
    {0x10, "KeyShift", L"Shift"},
    {0x11, "KeyCtrl", L"Ctrl"},
    {0x12, "KeyAlt", L"Alt"},
    {0x13, 0, L"Pause"},
    {0x14, "KeyCapsLock", L"Caps Lock"},
    {0x1B, "KeyEscape", L"Esc"},
    {0x20, "KeySpace", L"Space"},
    {0x21, "KeyPageUp", L"Page Up"},
    {0x22, "KeyPageDown", L"Page Down"},
    {0x23, "KeyEnd", L"End"},
    {0x24, "KeyHome", L"Home"},
    {0x25, "KeyLeft", L"Left"},
    {0x26, "KeyUp", L"Up"},
    {0x27, "KeyRight", L"Right"},
    {0x28, "KeyDown", L"Down"},
    {0x2C, 0, L"Print Screen"},
    {0x2D, "KeyInsert", L"Insert"},
    {0x2E, "KeyDelete", L"Delete"},
    {0x6A, 0, L"Num *"},
    {0x6B, 0, L"Num +"},
    {0x6D, 0, L"Num -"},
    {0x6E, 0, L"Num ."},
    {0x6F, 0, L"Num /"},
    {0x90, 0, L"Num Lock"},
    {0x91, 0, L"Scroll Lock"},
    {0xBA, "KeySemicolon", L";"},
    {0xBB, "KeyEquals", L"="},
    {0xBC, "KeyComma", L","},
    {0xBD, "KeyMinus", L"-"},
    {0xBE, "KeyPeriod", L"."},
    {0xBF, "KeySlash", L"/"},
    {0xC0, "KeyGrave", L"`"},
    {0xDB, "KeyLeftBracket", L"["},
    {0xDC, "KeyBackslash", L"\\"},
    {0xDD, "KeyRightBracket", L"]"},
    {0xDE, "KeyApostrophe", L"'"},
    {kPcKeyMouseLeft, "MouseLeft", L"Left Click"},
    {kPcKeyMouseRight, "MouseRight", L"Right Click"},
    {kPcKeyMouseMiddle, "MouseMiddle", L"Middle Click"},
    {kPcKeyMouse4, "Mouse4", L"Mouse 4"},
    {kPcKeyMouse5, "Mouse5", L"Mouse 5"},
};

int codes[ePcKey_Count];
bool loaded = false;

struct ModKey
{
    std::string id;
    std::wstring label;
    int def;
    int code;
};
std::vector<ModKey> modKeys;
std::map<std::string, int> otherLines;

std::string BindingsPath()
{
    return PcSaveRoot() + "/" + BINDINGS_FILE;
}

bool ValidCode(int code)
{
    return (code > 0 && code < 0x100) || (code >= kPcKeyMouseFirst && code < kPcKeyMouseFirst + kPcKeyMouseCount);
}

void EnsureLoaded()
{
    if (!loaded)
        PcKeyBindingsLoad();
}
}

int PcKeyBindingCount()
{
    return ePcKey_Count + (int)modKeys.size();
}

int PcKeyBindingsAddMod(const std::string& id, const std::wstring& label, int code)
{
    EnsureLoaded();
    if (code != kPcKeyNone && !ValidCode(code))
        code = kPcKeyNone;
    for (size_t i = 0; i < modKeys.size(); i++)
        if (modKeys[i].id == id)
            return -1;
    ModKey k;
    k.id = id;
    k.label = label;
    k.def = code;
    k.code = code;
    std::map<std::string, int>::iterator it = otherLines.find(id);
    if (it != otherLines.end())
    {
        if (it->second == kPcKeyNone || ValidCode(it->second))
            k.code = it->second;
        otherLines.erase(it);
    }
    modKeys.push_back(k);
    return ePcKey_Count + (int)modKeys.size() - 1;
}

void PcKeyBindingsLoad()
{
    loaded = true;
    for (int act = 0; act < ePcKey_Count; ++act)
        codes[act] = kBindings[act].code;
    for (size_t i = 0; i < modKeys.size(); i++)
        modKeys[i].code = modKeys[i].def;
    otherLines.clear();
    FILE* fp = fopen(BindingsPath().c_str(), "r");
    if (fp == 0)
        return;
    char line[192];
    while (fgets(line, sizeof(line), fp) != 0)
    {
        char id[128];
        int code = 0;
        if (sscanf(line, "%127[^=]=%d", id, &code) != 2)
            continue;
        bool found = false;
        for (int act = 0; act < ePcKey_Count; ++act)
        {
            if (strcmp(id, kBindings[act].id) == 0)
            {
                codes[act] = code == kPcKeyNone || ValidCode(code) ? code : kBindings[act].code;
                found = true;
            }
        }
        for (size_t i = 0; i < modKeys.size(); i++)
        {
            if (modKeys[i].id == id)
            {
                modKeys[i].code = code == kPcKeyNone || ValidCode(code) ? code : modKeys[i].def;
                found = true;
            }
        }
        if (!found && strchr(id, ':') != 0)
            otherLines[id] = code;
    }
    fclose(fp);
}

void PcKeyBindingsSave()
{
    EnsureLoaded();
    FILE* fp = fopen(BindingsPath().c_str(), "w");
    if (fp == 0)
        return;
    for (int act = 0; act < ePcKey_Count; ++act)
        fprintf(fp, "%s=%d\n", kBindings[act].id, codes[act]);
    for (size_t i = 0; i < modKeys.size(); i++)
        fprintf(fp, "%s=%d\n", modKeys[i].id.c_str(), modKeys[i].code);
    for (std::map<std::string, int>::iterator it = otherLines.begin(); it != otherLines.end(); ++it)
        fprintf(fp, "%s=%d\n", it->first.c_str(), it->second);
    fclose(fp);
}

void PcKeyBindingsReset()
{
    EnsureLoaded();
    for (int act = 0; act < ePcKey_Count; ++act)
        codes[act] = kBindings[act].code;
    for (size_t i = 0; i < modKeys.size(); i++)
        modKeys[i].code = modKeys[i].def;
    loaded = true;
    PcKeyBindingsSave();
}

int PcKeyBinding(PcKeyAction act)
{
    EnsureLoaded();
    if (act >= ePcKey_Count && act < PcKeyBindingCount())
        return modKeys[act - ePcKey_Count].code;
    return act >= 0 && act < ePcKey_Count ? codes[act] : kPcKeyNone;
}

int PcKeyBindingDefault(PcKeyAction act)
{
    if (act >= ePcKey_Count && act < PcKeyBindingCount())
        return modKeys[act - ePcKey_Count].def;
    return act >= 0 && act < ePcKey_Count ? kBindings[act].code : kPcKeyNone;
}

void PcKeyBindingSet(PcKeyAction act, int code)
{
    EnsureLoaded();
    if (act < 0 || act >= PcKeyBindingCount() || (code != kPcKeyNone && !ValidCode(code)))
        return;
    if (act >= ePcKey_Count)
        modKeys[act - ePcKey_Count].code = code;
    else
        codes[act] = code;
    PcKeyBindingsSave();
}

PcKeySection PcKeyBindingSection(PcKeyAction act)
{
    if (act >= ePcKey_Count)
        return ePcKeySection_Mods;
    return kBindings[act].section;
}

std::wstring PcKeyBindingLabel(PcKeyAction act)
{
    if (act >= ePcKey_Count && act < PcKeyBindingCount())
        return modKeys[act - ePcKey_Count].label;
    if (kBindings[act].text != 0)
        return kBindings[act].text;
    return L10N::GetString(kBindings[act].label);
}

std::wstring PcKeySectionLabel(PcKeySection sec)
{
    switch (sec)
    {
    case ePcKeySection_Movement:
        return L10N::GetString(IDS_CONTROLS_SECTION_MOVEMENT);
    case ePcKeySection_Standard:
        return L10N::GetString(IDS_CONTROLS_SECTION_STANDARD);
    case ePcKeySection_Hotbar:
        return L"Hotbar";
    case ePcKeySection_Creative:
        return L10N::GetString(IDS_CONTROLS_SECTION_CREATIVE);
    case ePcKeySection_MiniGameLobby:
        return L10N::GetString(IDS_CONTROLS_SECTION_MG_LOBBY);
    case ePcKeySection_Spectating:
        return L10N::GetString(IDS_CONTROLS_SECTION_SPECTATING);
    case ePcKeySection_Glide:
        return L10N::GetString(IDS_CONTROLS_SECTION_GLIDE);
    case ePcKeySection_Mods:
        return L"Mods";
    default:
        return std::wstring();
    }
}

int PcKeyBindingGameAction(PcKeyAction act)
{
    if (act >= ePcKey_Count)
        return -1;
    return kBindings[act].gameAction;
}

bool PcKeyBindingsDefaultMovement()
{
    return PcKeyBinding(ePcKey_Forward) == 'W' && PcKeyBinding(ePcKey_Back) == 'S' && PcKeyBinding(ePcKey_Left) == 'A'
           && PcKeyBinding(ePcKey_Right) == 'D';
}

const char* PcKeyGlyphName(int code)
{
    static char name[16];
    if ((code >= 'A' && code <= 'Z') || (code >= '0' && code <= '9'))
    {
        snprintf(name, sizeof(name), "Key%c", static_cast<char>(code));
        return name;
    }
    if (code >= 0x70 && code <= 0x7B)
    {
        snprintf(name, sizeof(name), "KeyF%d", code - 0x6F);
        return name;
    }
    for (unsigned int index = 0; index < sizeof(kKeyNames) / sizeof(kKeyNames[0]); ++index)
    {
        if (kKeyNames[index].code == code)
            return kKeyNames[index].glyph;
    }
    return 0;
}

std::wstring PcKeyCodeName(int code)
{
    wchar_t text[32];
    if (code == kPcKeyNone)
        return L10N::GetString(IDS_CONTROLS_NONE);
    if ((code >= 'A' && code <= 'Z') || (code >= '0' && code <= '9'))
        return std::wstring(1, static_cast<wchar_t>(code));
    if (code >= 0x70 && code <= 0x87)
    {
        swprintf(text, 32, L"F%d", code - 0x6F);
        return text;
    }
    if (code >= 0x60 && code <= 0x69)
    {
        swprintf(text, 32, L"Num %d", code - 0x60);
        return text;
    }
    for (unsigned int index = 0; index < sizeof(kKeyNames) / sizeof(kKeyNames[0]); ++index)
    {
        if (kKeyNames[index].code == code)
            return kKeyNames[index].text;
    }
    swprintf(text, 32, L"Key %d", code);
    return text;
}

std::wstring PcKeyCodeMarkup(int code, int size)
{
    const char* glyph = PcKeyGlyphName(code);
    if (glyph == 0)
        return L" " + PcKeyCodeName(code);
    wchar_t markup[128];
    swprintf(markup, 128, L"<img src=\"PcKey_%s\" align=\"middle\" height=\"%d\" width=\"%d\"/>", glyph, size, size);
    return markup;
}

bool PcKeyActionMarkup(int controllerAction, int size, std::wstring* markup)
{
    if (controllerAction == EControllerActions_MoveRight)
    {
        if (PcKeyBindingsDefaultMovement())
        {
            wchar_t text[128];
            swprintf(text, 128, L"<img src=\"PcKey_KeyWASD\" align=\"middle\" height=\"%d\" width=\"%d\"/>", size, size);
            *markup = text;
        }
        else
        {
            *markup = PcKeyCodeMarkup(PcKeyBinding(ePcKey_Forward), size) + PcKeyCodeMarkup(PcKeyBinding(ePcKey_Left), size)
                      + PcKeyCodeMarkup(PcKeyBinding(ePcKey_Back), size) + PcKeyCodeMarkup(PcKeyBinding(ePcKey_Right), size);
        }
        return true;
    }
    if (controllerAction == EControllerActions_LookDown)
    {
        wchar_t text[128];
        swprintf(text, 128, L"<img src=\"PcKey_Mouse\" align=\"middle\" height=\"%d\" width=\"%d\"/>", size, size);
        *markup = text;
        return true;
    }
    if (controllerAction == EControllerActions_Sprint)
    {
        *markup = PcKeyCodeMarkup(PcKeyBinding(ePcKey_Sprint), size);
        return true;
    }
    for (int action = 0; action < ePcKey_Count; ++action)
    {
        if (kBindings[action].gameAction == controllerAction)
        {
            *markup = PcKeyCodeMarkup(PcKeyBinding(static_cast<PcKeyAction>(action)), size);
            return true;
        }
    }
    return false;
}

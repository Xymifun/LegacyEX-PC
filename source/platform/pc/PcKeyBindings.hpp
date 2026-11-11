#ifndef PLATFORM_PC_PCKEYBINDINGS_HPP
#define PLATFORM_PC_PCKEYBINDINGS_HPP

#include <string>

enum PcKeyAction
{
    ePcKey_Forward,
    ePcKey_Back,
    ePcKey_Left,
    ePcKey_Right,
    ePcKey_Jump,
    ePcKey_Sneak,
    ePcKey_Sprint,
    ePcKey_Use,
    ePcKey_Action,
    ePcKey_PickBlock,
    ePcKey_Drop,
    ePcKey_Inventory,
    ePcKey_Crafting,
    ePcKey_CycleLeft,
    ePcKey_CycleRight,
    ePcKey_ChangeCamera,
    ePcKey_PlayerList,
    ePcKey_Chat,
    ePcKey_Command,
    ePcKey_PushToTalk,
    ePcKey_Screenshot,
    ePcKey_Hotbar1,
    ePcKey_Hotbar2,
    ePcKey_Hotbar3,
    ePcKey_Hotbar4,
    ePcKey_Hotbar5,
    ePcKey_Hotbar6,
    ePcKey_Hotbar7,
    ePcKey_Hotbar8,
    ePcKey_Hotbar9,
    ePcKey_FlyLeft,
    ePcKey_FlyRight,
    ePcKey_FlyUp,
    ePcKey_FlyDown,
    ePcKey_ConfirmReady,
    ePcKey_Vote,
    ePcKey_SpectatePrevious,
    ePcKey_SpectateNext,
    ePcKey_SpectateNoise,
    ePcKey_SpectateCancel,
    ePcKey_RestartSection,
    ePcKey_RestartRace,
    ePcKey_LookBehind,
    ePcKey_GlideGhost,
    ePcKey_Count
};

enum PcKeySection
{
    ePcKeySection_Movement,
    ePcKeySection_Standard,
    ePcKeySection_Hotbar,
    ePcKeySection_Creative,
    ePcKeySection_MiniGameLobby,
    ePcKeySection_Spectating,
    ePcKeySection_Glide,
    ePcKeySection_Mods,
    ePcKeySection_Count
};

const int kPcKeyNone = 0;
const int kPcKeyMouseLeft = 0x101;
const int kPcKeyMouseRight = 0x102;
const int kPcKeyMouseMiddle = 0x103;
const int kPcKeyMouse4 = 0x104;
const int kPcKeyMouse5 = 0x105;
const int kPcKeyMouseFirst = kPcKeyMouseLeft;
const int kPcKeyMouseCount = 5;

int PcKeyBindingCount();
int PcKeyBindingsAddMod(const std::string& id, const std::wstring& label, int code);

void PcKeyBindingsLoad();
void PcKeyBindingsSave();
void PcKeyBindingsReset();

int PcKeyBinding(PcKeyAction action);
int PcKeyBindingDefault(PcKeyAction action);
void PcKeyBindingSet(PcKeyAction action, int code);
PcKeySection PcKeyBindingSection(PcKeyAction action);
std::wstring PcKeyBindingLabel(PcKeyAction action);
std::wstring PcKeySectionLabel(PcKeySection section);
int PcKeyBindingGameAction(PcKeyAction action);
bool PcKeyBindingsDefaultMovement();

const char* PcKeyGlyphName(int code);
std::wstring PcKeyCodeName(int code);
std::wstring PcKeyCodeMarkup(int code, int size);
bool PcKeyActionMarkup(int controllerAction, int size, std::wstring* markup);

#endif

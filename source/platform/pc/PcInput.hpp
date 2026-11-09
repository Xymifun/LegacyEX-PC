#ifndef PLATFORM_PC_PCINPUT_HPP
#define PLATFORM_PC_PCINPUT_HPP

#include "platform/pc/PcKeyBindings.hpp"

#include "platform/pc/PcOs.hpp"

#include <padscore/kpad.h>
#include <stdint.h>

#include <string>

struct VPADStatus;

enum PcGlyphSet
{
    ePcGlyphSet_Auto,
    ePcGlyphSet_WiiU,
    ePcGlyphSet_Keyboard,
    ePcGlyphSet_PS3,
    ePcGlyphSet_PS4,
    ePcGlyphSet_Xbox360,
    ePcGlyphSet_XboxOne
};

void PcInputHandleEvent(const PcOsWindowEvent& event);
bool PcInputActive();
void PcInputFillGamePad(VPADStatus* status);
bool PcInputFillProController(int channel, KPADStatus* status);
KPADConnectCallback PcInputSetConnectCallback(int channel, KPADConnectCallback callback);
void PcInputSetNintendoButtons(bool nintendo);
void PcInputSetBackgroundInput(bool background);
void PcInputSetInGame(bool inGame);
bool PcInputMenuPointer(float* x, float* y);
bool PcInputMenuPointerActive();
bool PcInputMenuCursor(int* windowX, int* windowY);
bool PcInputConsumeHover(float* x, float* y);
PcGlyphSet PcInputGlyphSet();
int PcInputGlyphContext();
bool PcInputNintendoButtons();
void PcInputNoteMouseLook(float x, float y);
bool PcInputRecentMouseLook(int direction);
void PcInputNoteHotbarWheel(int direction);
bool PcInputRecentHotbarWheel(int direction);
void PcInputSetGlyphSet(PcGlyphSet set);
void PcInputSetScriptedCursor(float x, float y);
void PcInputSetScriptedPad(unsigned int buttons);

struct PcInputModPad
{
    unsigned int hold;
    float leftX;
    float leftY;
    float rightX;
    float rightY;
    bool pointer;
    float pointerX;
    float pointerY;
    bool press;
    bool chat;
    bool sneak;
};
void PcInputSetModPad(const PcInputModPad& pad);
bool PcInputPointerContext();
int PcInputConsumeHotbarKey();
int PcInputConsumeContainerHotbarKey();
int PcInputConsumeContainerWheel();
bool PcInputContainerLeftHeld();
int PcInputConsumeHotbarWheel();
void PcInputSetGlideKeys(bool active);
bool PcInputGlideKeys();
void PcInputSetMiniGameKeys(bool lobby, bool spectating, bool glide);
bool PcInputKeySectionActive(PcKeySection section);
bool PcInputConsumeDebugCameraKey();
bool PcInputConsumeDebugFillAreaKey();
bool PcInputConsumeDebugSchematicKey();
char16_t PcInputConsumeChatKey();
void PcInputSetChatOpen(bool open);
bool PcInputSprintHeld();
bool PcInputKeyboardStrafing();
bool PcInputSneakHeld();
bool PcInputConsumeScreenshotKey();
int PcInputConsumeScriptKey();
bool PcInputPushToTalkHeld();
bool PcInputBindingHeld(PcKeyAction action);
uint32_t PcInputActionButtons(int controllerAction);
void PcInputBeginKeyCapture();
void PcInputCancelKeyCapture();
bool PcInputKeyCapturing();
int PcInputTakeKeyCapture();
bool PcInputKeyHeld(int key);
void PcInputSetDirectLook(bool direct);
void PcInputConsumeMouseDelta(float* x, float* y);
void PcInputSetLogging(bool log);
bool PcInputLogging();

enum PcTextEntryResult
{
    ePcTextEntry_Pending,
    ePcTextEntry_Accepted,
    ePcTextEntry_Cancelled
};

void PcTextEntryBegin(const char16_t* initialText, unsigned int maxLength);
void PcTextEntryEnd();
bool PcTextEntryActive();
const char16_t* PcTextEntryText();
void PcTextEntryType(char16_t character);
void PcTextEntryBackspace();
void PcTextEntryDelete();
void PcTextEntryMoveCursor(int direction, bool word);
void PcTextEntryPaste();
void PcTextEntrySetText(const std::u16string& text);
unsigned int PcTextEntryCursor();
int PcTextEntryConsumeHistoryKey();
void PcTextEntrySetResult(PcTextEntryResult result);
PcTextEntryResult PcTextEntryCurrentResult();
unsigned int PcTextEntryKeyboardTyping();

#endif

#ifndef PLATFORM_PC_PCTEXTKEYBOARD_HPP
#define PLATFORM_PC_PCTEXTKEYBOARD_HPP

#include "platform/pc/PcInput.hpp"

#include <stdint.h>

void PcTextKeyboardBegin();
PcTextEntryResult PcTextKeyboardUpdate(uint32_t hold, uint32_t trigger, float stickX, float stickY);
void PcTextKeyboardDraw();

class Font;
class Gui;

void PcTextEntryDrawLine(Font* font, Gui* gui, int x, int y, int width, int color, bool caretOn);

#endif

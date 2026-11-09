#ifndef PLATFORM_PC_PCCHAT_HPP
#define PLATFORM_PC_PCCHAT_HPP

#include <string>

class Minecraft;

void PcChatTick(Minecraft* minecraft, int pad, bool gameActive);
bool PcChatActive();
std::wstring PcChatText();

#endif

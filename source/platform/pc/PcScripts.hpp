#ifndef PLATFORM_PC_PCSCRIPTS_HPP
#define PLATFORM_PC_PCSCRIPTS_HPP

#include <string>
#include <vector>

class Block;
class Minecraft;
class ServerPlayer;
class ServerLevel;
class BlockPos;

void PcScriptsClientLoad();
void PcScriptsClientFrame(Minecraft* minecraft);
bool PcScriptsClientChat(std::wstring* message);
bool PcScriptsClientCommand(const std::wstring& line);

struct PcScriptHudItem
{
    bool text;
    bool shadow;
    int x1;
    int y1;
    int x2;
    int y2;
    unsigned int colour;
    std::wstring string;
};

bool PcScriptsClientHasHud();
void PcScriptsClientHud(int width, int height, std::vector<PcScriptHudItem>* items);

void PcScriptsHostStart();
void PcScriptsHostStop();
void PcScriptsHostTick();
void PcScriptsHostPlayerJoined(ServerPlayer* player);
void PcScriptsHostPlayerLeaving(ServerPlayer* player);
bool PcScriptsHostChat(ServerPlayer* player, std::wstring* message);
bool PcScriptsHostCommand(ServerPlayer* player, const std::wstring& line);
bool PcScriptsHostBlockBreak(ServerPlayer* player, ServerLevel* level, const BlockPos& pos);
bool PcScriptsHostBlockPlace(ServerPlayer* player, ServerLevel* level, const BlockPos& pos, Block* block, int data);

#endif

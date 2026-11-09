// An example native mod (tools/pc/README.md, Native mods). Build it with the game:
//   ninja -C build/pc64 pc_native_example
// then copy build/pc64/native-example/win64.dll to mods/native-example/native/win64.dll, next to a
// mod.json ({"id": "native-example", "side": "client", "api": 2}).
//
// It calls the game directly: Minecraft::GetInstance, the server's player list and a level's name
// (a std::wstring the game makes and this mod frees, from the one heap they share).

#include "PcGameBuild.h"
#include "platform/pc/PcNativeMod.h"

#include <stdio.h>
#include <string>

#include "game/core/Minecraft.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/PlayerList.hpp"

namespace
{
const PcNativeHost* s_host = 0;
unsigned long long s_frames = 0;
unsigned long long s_ticks = 0;

// a static constructor: runs when the game starts the mod's C runtime
std::string s_greeting = std::string("hello from native code, built for game ") + PC_GAME_BUILD;

std::string Narrow(const std::wstring& text)
{
    std::string narrow;
    for (size_t i = 0; i < text.size(); ++i)
        narrow += text[i] < 0x80 ? static_cast<char>(text[i]) : '?';
    return narrow;
}

void Log(const std::string& text)
{
    s_host->log(text.c_str());
}

void Frame(void*)
{
    ++s_frames;
    if (s_frames % 120 != 0)
        return;
    Minecraft* minecraft = Minecraft::GetInstance();
    MultiplayerLocalPlayer* player = minecraft != 0 ? minecraft->mLocalPlayers[0].get() : 0;
    char text[128];
    if (player != 0)
        snprintf(text, sizeof(text), "frame %llu: the player is at %.1f %.1f %.1f", s_frames, player->mX, player->mY,
                 player->mZ);
    else
        snprintf(text, sizeof(text), "frame %llu: not in a world", s_frames);
    Log(text);
}

void HostStart(void*)
{
    ServerLevel* level = gMinecraftServer->getLevel(0);
    Log("a world started: \"" + Narrow(level->getLevelData()->getLevelName()) + "\"");
}

void HostTick(void*)
{
    if (++s_ticks % 200 != 0)
        return;
    char text[96];
    snprintf(text, sizeof(text), "server tick %llu: %d player(s), day time %lld", s_ticks,
             static_cast<int>(gMinecraftServer->getPlayers()->getPlayers()->size()), gMinecraftServer->getLevel(0)->getDayTime());
    Log(text);
}

void HostStop(void*)
{
    Log("the world stopped");
}
}

PC_NATIVE_MOD_RECORD
{
    s_host = host;
    Log(s_greeting);
    char text[160];
    snprintf(text, sizeof(text), "mod %s in %s; Minecraft::GetInstance is at %p, found by name at %p", host->modId,
             host->modDirectory, reinterpret_cast<void*>(&Minecraft::GetInstance),
             host->findSymbol("_ZN9Minecraft11GetInstanceEv"));
    Log(text);
    host->onFrame(Frame, 0);
    host->onHostStart(HostStart, 0);
    host->onHostTick(HostTick, 0);
    host->onHostStop(HostStop, 0);
    return 0;
}

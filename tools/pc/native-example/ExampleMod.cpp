// An example native mod (tools/pc/README.md, Native mods). Build it with the game:
//   ninja -C build/pc64 pc_native_example
// then copy build/pc64/native-example/win64.dll to mods/native-example/native/win64.dll, next to a
// mod.json ({"id": "native-example", "side": "both", "api": 2}) and a 16x16 textures/example_block.png.
// A mod that adds content must be "both": every player in a world needs it.
//
// It calls the game directly: Minecraft::GetInstance, the server's player list and a level's name
// (a std::wstring the game makes and this mod frees, from the one heap they share). It also shows the
// content hooks (tools/pc/README.md, Native content hooks): a block and a mob type registered while the
// game starts, the mob spawned next to the first player, a chunk callback, and a message sent from the
// player's game to the host and answered.

#include "PcGameBuild.h"
#include "platform/pc/PcNativeMod.h"

#include <stdio.h>
#include <string>
#include <vector>

#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/ZombieRenderer.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Zombie.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/PlayerList.hpp"

namespace
{
const char kMessageChannel[] = "native-example";

const PcNativeHost* s_host = 0;
unsigned long long s_frames = 0;
unsigned long long s_ticks = 0;
const wchar_t* s_blockSprite = L"";
unsigned long long s_mobType = 0;
bool s_mobSpawned = false;
bool s_messageSent = false;
int s_decoratedChunks = 0;

// A plain cube. Every block class builds its state definition in its constructor (DerivedInit).
class ExampleBlock : public Block
{
public:
    ExampleBlock() : Block(Material::STONE)
    {
        DerivedInit();
        setIconName(s_blockSprite);
        setDestroyTime(1.5f);
    }
};

// A zombie with a type of its own: the game's "is it a monster" checks still see it as one, because the
// type was registered under EntityType::MONSTER. GetType must return the registered type.
class ExampleMob : public Zombie
{
public:
    explicit ExampleMob(Level* level) : Zombie(level) {}
    virtual unsigned long long GetType() { return s_mobType; }
};

void* MakeBlock(int)
{
    return new ExampleBlock();
}

void* MakeMob(void* level)
{
    return new ExampleMob(static_cast<Level*>(level));
}

void* MakeMobRenderer(void* dispatcher)
{
    return new ZombieRenderer(static_cast<EntityRenderDispatcher*>(dispatcher));
}

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
    Minecraft* game = Minecraft::GetInstance();
    if (!s_messageSent && game != 0 && game->mPlayer != 0 && s_frames > 600)
    {
        // from the player's game to the host; the host answers in Message
        s_messageSent = true;
        s_host->sendToHost(kMessageChannel, "hello", 5);
    }
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
    ++s_ticks;
    std::vector<boost::shared_ptr<ServerPlayer> >* players = gMinecraftServer->getPlayers()->getPlayers();
    if (!s_mobSpawned && s_ticks >= 100 && !players->empty())
    {
        s_mobSpawned = true;
        boost::shared_ptr<ServerPlayer> player = players->back();
        boost::shared_ptr<Entity> mob(new ExampleMob(player->mLevel));
        mob->moveTo(player->mX + 3.0, player->mY, player->mZ, 0.0f, 0.0f);
        player->mLevel->addEntity(mob);
        Log("spawned an example mob next to the player");
    }
    if (s_ticks % 200 != 0)
        return;
    char text[96];
    snprintf(text, sizeof(text), "server tick %llu: %d player(s), day time %lld", s_ticks,
             static_cast<int>(gMinecraftServer->getPlayers()->getPlayers()->size()), gMinecraftServer->getLevel(0)->getDayTime());
    Log(text);
}

void HostStop(void*)
{
    char text[96];
    snprintf(text, sizeof(text), "the world stopped; %d new chunks were decorated while it ran", s_decoratedChunks);
    Log(text);
}

// After the game decorated a new chunk, on the thread that made it: the place for structures and
// features of the mod's own (Level::setBlock with positions inside chunk x, z).
void ChunkDecorated(void*, int, int, int, void*)
{
    ++s_decoratedChunks;
}

// On the host, player is the ServerPlayer that sent the message; on a player's game it is 0.
void Message(void* player, const char* channel, const void* data, int size, void*)
{
    const std::string text(static_cast<const char*>(data), static_cast<size_t>(size));
    Log(std::string(player != 0 ? "the host" : "the player's game") + " received \"" + text + "\" on " + channel);
    if (player != 0)
        s_host->sendToPlayer(player, kMessageChannel, "hello back", 10);
}
}

// Optional, called while the game starts, before it makes its blocks, items and entity types.
extern "C" __declspec(dllexport) int PcNativeModRegister(const PcNativeRegister* registry)
{
    s_blockSprite = registry->addTexture(PC_NATIVE_ATLAS_BLOCKS, "example_block");
    registry->addBlock("example_block", "Example Block", PC_NATIVE_TAB_BUILDING, MakeBlock);
    s_mobType = registry->addEntity("example_mob", EntityType::MONSTER, MakeMob);
    registry->addEntityRenderer(s_mobType, MakeMobRenderer);
    return 0;
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
    host->onChunkDecorated(ChunkDecorated, 0);
    host->onMessage(Message, 0);
    return 0;
}

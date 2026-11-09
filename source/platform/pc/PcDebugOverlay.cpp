#include "platform/pc/PcDebugOverlay.hpp"

#include "platform/pc/PcDisplay.hpp"
#include "platform/pc/PcOs.hpp"

#include "game/core/Minecraft.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <wchar.h>

namespace
{
const unsigned long long kFpsWindowNanoseconds = 1000000000ULL;

bool s_visible = false;
unsigned long long s_fpsWindowStart = 0;
int s_fpsFrames = 0;
int s_fps = 0;

int FloorToInt(double value)
{
    return static_cast<int>(floor(value));
}

std::wstring Format(const wchar_t* format, ...)
{
    wchar_t text[256];
    va_list arguments;
    va_start(arguments, format);
    vswprintf(text, sizeof(text) / sizeof(text[0]), format, arguments);
    va_end(arguments);
    return std::wstring(text);
}

void Facing(float yaw, const wchar_t** name, const wchar_t** towards)
{
    static const wchar_t* const kNames[] = {L"south", L"west", L"north", L"east"};
    static const wchar_t* const kTowards[] = {L"positive Z", L"negative X", L"negative Z", L"positive X"};
    int index = FloorToInt(yaw / 90.0f + 0.5f) & 3;
    *name = kNames[index];
    *towards = kTowards[index];
}

float WrapDegrees(float degrees)
{
    degrees = fmodf(degrees, 360.0f);
    if (degrees >= 180.0f)
        degrees -= 360.0f;
    if (degrees < -180.0f)
        degrees += 360.0f;
    return degrees;
}

const wchar_t* DimensionName(int id)
{
    if (id == -1)
        return L"the_nether";
    if (id == 1)
        return L"the_end";
    return L"overworld";
}
}

void PcDebugOverlayToggle()
{
    s_visible = !s_visible;
}

void PcDebugOverlayFrame()
{
    unsigned long long now = PcOsMonotonicNanoseconds();
    if (s_fpsWindowStart == 0)
        s_fpsWindowStart = now;
    ++s_fpsFrames;
    if (now - s_fpsWindowStart >= kFpsWindowNanoseconds)
    {
        s_fps = static_cast<int>(static_cast<unsigned long long>(s_fpsFrames) * kFpsWindowNanoseconds
                                 / (now - s_fpsWindowStart));
        s_fpsFrames = 0;
        s_fpsWindowStart = now;
    }
}

bool PcDebugOverlayLines(Minecraft* minecraft, std::vector<std::wstring>* left, std::vector<std::wstring>* right)
{
    if (!s_visible || minecraft == 0 || minecraft->mPlayer == 0 || minecraft->mLevel == 0)
        return false;
    MultiplayerLocalPlayer* player = minecraft->mPlayer.get();
    Level* level = minecraft->mLevel;
    const BlockPos block(FloorToInt(player->mX), FloorToInt(player->mY), FloorToInt(player->mZ));
    const wchar_t* facing;
    const wchar_t* towards;
    Facing(player->mYRot, &facing, &towards);

    left->push_back(L"Minecraft Legacy Edition (PC port)");
    left->push_back(Format(L"%d fps", s_fps));
    left->push_back(L"");
    left->push_back(Format(L"XYZ: %.3f / %.5f / %.3f", player->mX, player->mY, player->mZ));
    left->push_back(Format(L"Block: %d %d %d", block.x, block.y, block.z));
    left->push_back(Format(L"Chunk: %d %d %d in %d %d %d", block.x & 15, block.y & 15, block.z & 15, block.x >> 4,
                           block.y >> 4, block.z >> 4));
    left->push_back(Format(L"Facing: %ls (Towards %ls) (%.1f / %.1f)", facing, towards, WrapDegrees(player->mYRot),
                           player->mXRot));
    Biome* biome = level->getBiome(block);
    if (biome != 0)
        left->push_back(Format(L"Biome: %ls", biome->getName(false).c_str()));
    left->push_back(Format(L"Light: %d (%d sky, %d block)", level->getRawBrightness(block),
                           level->getBrightness(LightLayer::SKY, block), level->getBrightness(LightLayer::BLOCK, block)));
    left->push_back(Format(L"Dimension: %ls", DimensionName(level->mDimension->getType()->getId())));
    left->push_back(Format(L"E: %u", static_cast<unsigned int>(level->getAllEntities().size())));
    left->push_back(Format(L"Players: %d", CGameNetworkManager::sInstance.GetPlayerCount()));

    int renderWidth = 0;
    int renderHeight = 0;
    PcGetRenderSize(&renderWidth, &renderHeight);
    right->push_back(sizeof(void*) == 8 ? L"Windows build (64-bit)" : L"Windows build (32-bit)");
    right->push_back(Format(L"Display: %dx%d", renderWidth, renderHeight));
    const char* renderer = PcGetGlRendererName();
    if (renderer != 0 && renderer[0] != 0)
        right->push_back(Format(L"%hs", renderer));
    return true;
}

#include "platform/pc/PcGoldenAge.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/LevelRenderer.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/client/render/Textures.hpp"
#include "game/client/render/TextureUtil.hpp"
#include "game/core/BufferedImage.hpp"
#include "game/core/Minecraft.hpp"
#include "platform/pc/PcPaths.hpp"
#include "platform/pc/PcSettings.hpp"

namespace
{
const char* const kFolder = "PC/GoldenAge/";
const char* const kRequiredFiles[] = {"res/terrain.png",   "res/gui/items.png",     "res/armor/cloth_1.png",
                                      "res/armor/cloth_2.png", "res/mob/pig.png", "sound/random_hurt.wav",
                                      "sound/random_bow.wav"};
const int kAtlasColumns = 16;
const unsigned int kHiddenAlpha = 16;

struct Cell
{
    int row;
    int column;
    int sourceRow;
    int sourceColumn;
};

const Cell kBlockCells[] = {{0, 7, 0, 7},  {0, 12, 0, 12}, {1, 0, 1, 0},  {1, 3, 1, 3},
                            {1, 6, 1, 6}, {2, 4, 2, 4},   {2, 14, 2, 14}};
const Cell kItemCells[] = {{5, 8, 5, 8}, {5, 9, 5, 9}, {5, 10, 5, 10}, {10, 8, 0, 7},
                           {0, 0, 0, 0}, {1, 0, 1, 0}, {2, 0, 2, 0},   {3, 0, 3, 0}};
const Cell kItemClearedCells[] = {{9, 0, 0, 0}, {10, 0, 0, 0}, {11, 0, 0, 0}, {12, 0, 0, 0}};

struct WaterRecolour
{
    const wchar_t* path;
    int scale[4];
    int offset[4];
};

const WaterRecolour kWaterRecolours[] = {
    {L"textures/blocks/water", {0, 232, 494, 0}, {212000, -19890, -47944, 182600}},
    {L"textures/blocks/water_flow", {0, 133, 293, 0}, {212000, -2530, -12680, 183400}},
    {L"textures/blocks/cauldron_water", {0, 232, 494, 0}, {212000, -19890, -47944, 182600}},
};

struct ImageOverride
{
    const wchar_t* name;
    const char* file;
    int group;
};

const ImageOverride kImageOverrides[] = {
    {L"armor/cloth_1", "res/armor/cloth_1.png", kPcGoldenAgeItems},
    {L"armor/cloth_2", "res/armor/cloth_2.png", kPcGoldenAgeItems},
    {L"armor/cloth_1_b", 0, kPcGoldenAgeItems},
    {L"armor/cloth_2_b", 0, kPcGoldenAgeItems},
    {L"mob/pig", "res/mob/pig.png", kPcGoldenAgeModels},
};

const wchar_t* const kLavaStill = L"textures/blocks/lava";
const wchar_t* const kLavaFlow = L"textures/blocks/lava_flow";
const int kLavaFrames = 96;
const int kLavaCrossfade = 24;
const int kLavaWarmup = 400;
const int kLavaCells = 256;

struct LavaSimulation
{
    float current[kLavaCells];
    float next[kLavaCells];
    float heat[kLavaCells];
    float heatRate[kLavaCells];
    unsigned int seed;

    LavaSimulation() : seed(0x4C415641u)
    {
        memset(current, 0, sizeof(current));
        memset(next, 0, sizeof(next));
        memset(heat, 0, sizeof(heat));
        memset(heatRate, 0, sizeof(heatRate));
    }

    float Random()
    {
        seed = seed * 1103515245u + 12345u;
        return static_cast<float>((seed >> 8) & 0xFFFFFF) / 16777216.0f;
    }

    void Tick()
    {
        const float pi = 3.14159265f;
        for (int x = 0; x < 16; ++x)
        {
            for (int y = 0; y < 16; ++y)
            {
                const int xOffset = static_cast<int>(sinf(static_cast<float>(y) * pi * 2.0f / 16.0f) * 1.2f);
                const int yOffset = static_cast<int>(sinf(static_cast<float>(x) * pi * 2.0f / 16.0f) * 1.2f);
                float sum = 0.0f;
                for (int xx = x - 1; xx <= x + 1; ++xx)
                    for (int yy = y - 1; yy <= y + 1; ++yy)
                        sum += current[((xx + xOffset) & 15) + ((yy + yOffset) & 15) * 16];
                const int cell = x + y * 16;
                next[cell] = sum / 10.0f
                             + (heat[(x & 15) + (y & 15) * 16] + heat[((x + 1) & 15) + (y & 15) * 16]
                                + heat[((x + 1) & 15) + ((y + 1) & 15) * 16] + heat[(x & 15) + ((y + 1) & 15) * 16])
                                   / 4.0f * 0.8f;
                heat[cell] += heatRate[cell] * 0.01f;
                if (heat[cell] < 0.0f)
                    heat[cell] = 0.0f;
                heatRate[cell] -= 0.06f;
                if (Random() < 0.005f)
                    heatRate[cell] = 1.5f;
            }
        }
        memcpy(current, next, sizeof(current));
    }
};

int LavaPixel(float value)
{
    float power = value * 2.0f;
    power = power < 0.0f ? 0.0f : (power > 1.0f ? 1.0f : power);
    const int red = static_cast<int>(power * 100.0f + 155.0f);
    const int green = static_cast<int>(power * power * 255.0f);
    const int blue = static_cast<int>(power * power * power * power * 128.0f);
    return static_cast<int>(0xFF000000u | (static_cast<unsigned int>(red) << 16)
                            | (static_cast<unsigned int>(green) << 8) | static_cast<unsigned int>(blue));
}

BufferedImage* MakeLavaAnimation(bool flowing)
{
    std::vector<float> fields(static_cast<size_t>(kLavaFrames + kLavaCrossfade) * kLavaCells);
    LavaSimulation simulation;
    for (int tick = 0; tick < kLavaWarmup; ++tick)
        simulation.Tick();
    for (int frame = 0; frame < kLavaFrames + kLavaCrossfade; ++frame)
    {
        simulation.Tick();
        memcpy(&fields[static_cast<size_t>(frame) * kLavaCells], simulation.current, sizeof(simulation.current));
    }
    const int size = flowing ? 32 : 16;
    BufferedImage* image = new BufferedImage(size, size * kLavaFrames, 1);
    int* pixels = image->getData();
    for (int frame = 0; frame < kLavaFrames; ++frame)
    {
        const float* field = &fields[static_cast<size_t>(frame) * kLavaCells];
        const bool crossfade = frame < kLavaCrossfade;
        const float* wrapped = crossfade ? &fields[static_cast<size_t>(frame + kLavaFrames) * kLavaCells] : field;
        const float blend = crossfade ? static_cast<float>(frame) / kLavaCrossfade : 1.0f;
        const int scroll = flowing ? (frame / 3) * 16 : 0;
        for (int y = 0; y < size; ++y)
        {
            for (int x = 0; x < size; ++x)
            {
                const int cell = (((y & 15) * 16 + (x & 15)) - scroll) & (kLavaCells - 1);
                const float value = field[cell] * blend + wrapped[cell] * (1.0f - blend);
                pixels[(frame * size + y) * size + x] = LavaPixel(value);
            }
        }
    }
    return image;
}

int s_installed = -1;
int s_terrainTexture = -1;
std::vector<char> s_hurtSound;
std::vector<char> s_bowSound;
std::vector<char> s_doorOpenSound;
std::vector<char> s_doorCloseSound;

std::string GamePath(const char* file)
{
    return std::string(kFolder) + file;
}

bool FileExists(const char* file)
{
    FILE* handle = fopen(PcResolvePath(GamePath(file).c_str()).c_str(), "rb");
    if (handle == 0)
        return false;
    fclose(handle);
    return true;
}

bool DefaultPackSelected()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft == 0 || minecraft->mTexturePackRepository == 0)
        return true;
    return minecraft->mTexturePackRepository->getSelected() == TexturePackRepository::defaultTexturePack;
}

bool LoadPixels(const char* file, int* width, int* height, int** pixels)
{
    *pixels = 0;
    if (TextureUtil::LoadTextureData(GamePath(file).c_str(), width, height, pixels) != 0 || *pixels == 0)
    {
        printf("[pc] golden age: could not read %s%s\n", kFolder, file);
        return false;
    }
    for (int pixel = 0; pixel < *width * *height; ++pixel)
    {
        if ((static_cast<unsigned int>((*pixels)[pixel]) >> 24) < kHiddenAlpha)
            (*pixels)[pixel] = 0;
    }
    return true;
}

void LoadSound(const char* file, std::vector<char>* data)
{
    FILE* handle = fopen(PcResolvePath(GamePath(file).c_str()).c_str(), "rb");
    if (handle == 0)
        return;
    fseek(handle, 0, SEEK_END);
    long size = ftell(handle);
    fseek(handle, 0, SEEK_SET);
    if (size > 0)
    {
        data->resize(static_cast<size_t>(size));
        if (fread(&(*data)[0], 1, data->size(), handle) != data->size())
            data->clear();
    }
    fclose(handle);
}

unsigned int AverageBlock(const int* source, int stride, int x, int y, int size)
{
    unsigned int sums[4] = {0, 0, 0, 0};
    for (int row = 0; row < size; ++row)
    {
        for (int column = 0; column < size; ++column)
        {
            unsigned int pixel = static_cast<unsigned int>(source[(y + row) * stride + x + column]);
            for (int channel = 0; channel < 4; ++channel)
                sums[channel] += (pixel >> (channel * 8)) & 0xFF;
        }
    }
    unsigned int count = static_cast<unsigned int>(size * size);
    unsigned int result = 0;
    for (int channel = 0; channel < 4; ++channel)
        result |= ((sums[channel] + count / 2) / count) << (channel * 8);
    return result;
}

void CopyCell(BufferedImage* image, const Cell& cell, const int* source, int sourceWidth, int sourceTile)
{
    const int width = static_cast<int>(image->getWidth());
    const int tile = width / kAtlasColumns;
    for (int level = 0; level < 10; ++level)
    {
        int* destination = image->getData(level);
        const int levelTile = tile >> level;
        if (destination == 0 || levelTile <= 0)
            break;
        const int levelWidth = width >> level;
        const int scale = sourceTile / levelTile;
        if (scale <= 0)
            break;
        for (int y = 0; y < levelTile; ++y)
        {
            for (int x = 0; x < levelTile; ++x)
            {
                int pixel = 0;
                if (source != 0)
                    pixel = static_cast<int>(AverageBlock(source, sourceWidth, cell.sourceColumn * sourceTile + x * scale,
                                                          cell.sourceRow * sourceTile + y * scale, scale));
                destination[(cell.row * levelTile + y) * levelWidth + cell.column * levelTile + x] = pixel;
            }
        }
    }
}
}

bool PcGoldenAgeInstalled()
{
    if (s_installed < 0)
    {
        s_installed = 1;
        for (unsigned int index = 0; index < sizeof(kRequiredFiles) / sizeof(kRequiredFiles[0]); ++index)
        {
            if (!FileExists(kRequiredFiles[index]))
                s_installed = 0;
        }
        if (s_installed != 0)
        {
            LoadSound("sound/random_hurt.wav", &s_hurtSound);
            LoadSound("sound/random_bow.wav", &s_bowSound);
            LoadSound("sound/random_door_open.wav", &s_doorOpenSound);
            LoadSound("sound/random_door_close.wav", &s_doorCloseSound);
        }
        printf("[pc] golden age: %s\n", s_installed != 0 ? "TU0 files found" : "no TU0 files in PC/GoldenAge");
    }
    return s_installed != 0;
}

bool PcGoldenAgeOn(int group)
{
    return (PcSettingsGoldenAge() & group) != 0 && PcGoldenAgeInstalled() && DefaultPackSelected();
}

void PcGoldenAgeSetGroups(int groups)
{
    const int previous = PcSettingsGoldenAge();
    if (groups == previous)
        return;
    PcSettingsSetGoldenAge(groups);
    PcSettingsSave();
    PcGoldenAgeRefresh(previous);
}

void PcGoldenAgeRefresh(int previousGroups)
{
    const int changed = previousGroups ^ PcSettingsGoldenAge();
    Minecraft* minecraft = Minecraft::GetInstance();
    if (changed == 0 || !PcGoldenAgeInstalled() || !DefaultPackSelected() || minecraft == 0)
        return;
    if ((changed & (kPcGoldenAgeBlocks | kPcGoldenAgeWater | kPcGoldenAgeLava | kPcGoldenAgeItems | kPcGoldenAgeModels))
            != 0
        && minecraft->mTextures != 0)
        minecraft->mTextures->reloadAll();
    if ((changed & (kPcGoldenAgeBlocks | kPcGoldenAgeWater)) != 0 && minecraft->mLevel != 0
        && minecraft->mLevelRenderer != 0)
        minecraft->mLevelRenderer->allChanged();
}

void PcGoldenAgePatchAtlas(int iconType, BufferedImage* image)
{
    const int group = iconType == 0 ? kPcGoldenAgeBlocks : kPcGoldenAgeItems;
    if (image == 0 || !PcGoldenAgeOn(group))
        return;
    int width = 0;
    int height = 0;
    int* pixels = 0;
    if (!LoadPixels(iconType == 0 ? "res/terrain.png" : "res/gui/items.png", &width, &height, &pixels))
        return;
    const int sourceTile = width / kAtlasColumns;
    if (iconType == 0)
    {
        for (unsigned int index = 0; index < sizeof(kBlockCells) / sizeof(kBlockCells[0]); ++index)
            CopyCell(image, kBlockCells[index], pixels, width, sourceTile);
    }
    else
    {
        for (unsigned int index = 0; index < sizeof(kItemCells) / sizeof(kItemCells[0]); ++index)
            CopyCell(image, kItemCells[index], pixels, width, sourceTile);
        for (unsigned int index = 0; index < sizeof(kItemClearedCells) / sizeof(kItemClearedCells[0]); ++index)
            CopyCell(image, kItemClearedCells[index], 0, width, sourceTile);
    }
    free(pixels);
}

void PcGoldenAgeRecolourAnimation(const std::wstring& path, BufferedImage* image)
{
    if (image == 0 || !PcGoldenAgeOn(kPcGoldenAgeWater))
        return;
    for (unsigned int index = 0; index < sizeof(kWaterRecolours) / sizeof(kWaterRecolours[0]); ++index)
    {
        const WaterRecolour& recolour = kWaterRecolours[index];
        if (path != recolour.path)
            continue;
        for (int level = 0; level < 10; ++level)
        {
            int* pixels = image->getData(level);
            const int width = static_cast<int>(image->getWidth()) >> level;
            const int height = static_cast<int>(image->getHeight()) >> level;
            if (pixels == 0 || width <= 0 || height <= 0)
                break;
            for (int pixel = 0; pixel < width * height; ++pixel)
            {
                const int grey = (static_cast<unsigned int>(pixels[pixel]) >> 16) & 0xFF;
                unsigned int result = 0;
                for (int channel = 0; channel < 4; ++channel)
                {
                    int value = (recolour.scale[channel] * grey + recolour.offset[channel]) / 1000;
                    value = value < 0 ? 0 : (value > 255 ? 255 : value);
                    result |= static_cast<unsigned int>(value) << (24 - channel * 8);
                }
                pixels[pixel] = static_cast<int>(result);
            }
        }
    }
}

BufferedImage* PcGoldenAgeReplaceAnimation(const std::wstring& path, BufferedImage* image)
{
    if ((path != kLavaStill && path != kLavaFlow) || !PcGoldenAgeOn(kPcGoldenAgeLava))
        return image;
    delete image;
    return MakeLavaAnimation(path == kLavaFlow);
}

void PcGoldenAgeAnimationFrames(const std::wstring& path, std::wstring* animation)
{
    if ((path != kLavaStill && path != kLavaFlow) || !PcGoldenAgeOn(kPcGoldenAgeLava))
        return;
    animation->clear();
    char entry[16];
    for (int frame = 0; frame < kLavaFrames; ++frame)
    {
        snprintf(entry, sizeof(entry), frame == 0 ? "%d*1" : ",%d*1", frame);
        for (const char* character = entry; *character != 0; ++character)
            *animation += static_cast<wchar_t>(*character);
    }
}

BufferedImage* PcGoldenAgeReadImage(const std::wstring& name)
{
    for (unsigned int index = 0; index < sizeof(kImageOverrides) / sizeof(kImageOverrides[0]); ++index)
    {
        const ImageOverride& entry = kImageOverrides[index];
        if (name != entry.name || !PcGoldenAgeOn(entry.group))
            continue;
        int width = 64;
        int height = 32;
        int* pixels = 0;
        if (entry.file != 0 && !LoadPixels(entry.file, &width, &height, &pixels))
            return 0;
        BufferedImage* image = new BufferedImage(width, height, 1);
        if (pixels != 0)
            memcpy(image->getData(), pixels, static_cast<size_t>(width) * height * sizeof(int));
        else
            memset(image->getData(), 0, static_cast<size_t>(width) * height * sizeof(int));
        free(pixels);
        return image;
    }
    return 0;
}

int PcGoldenAgeTerrainTexture()
{
    if (s_terrainTexture >= 0)
        return s_terrainTexture;
    int width = 0;
    int height = 0;
    int* pixels = 0;
    if (!LoadPixels("res/terrain.png", &width, &height, &pixels))
        return -1;
    BufferedImage* image = new BufferedImage(width, height, 1);
    memcpy(image->getData(), pixels, static_cast<size_t>(width) * height * sizeof(int));
    free(pixels);
    s_terrainTexture = Minecraft::GetInstance()->mTextures->getTexture(image, C4JRender::eTextureFormat_RGBA8, false);
    return s_terrainTexture;
}

const char* PcGoldenAgeEventName(const SoundEvent* sound)
{
    if (sound == SoundEvents::GENERIC_EXPLODE && PcGoldenAgeOn(kPcGoldenAgeSounds))
        return "random/old_explode";
    return 0;
}

const std::vector<char>* PcGoldenAgeSoundFile(const SoundEvent* sound)
{
    if (sound == 0 || !PcGoldenAgeOn(kPcGoldenAgeSounds))
        return 0;
    const std::vector<char>* file = 0;
    if (sound == SoundEvents::PLAYER_HURT || sound == SoundEvents::PLAYER_HURT_ON_FIRE
        || sound == SoundEvents::PLAYER_HURT_DROWN)
        file = &s_hurtSound;
    else if (sound == SoundEvents::ARROW_SHOOT)
        file = &s_bowSound;
    else if (sound == SoundEvents::WOODEN_DOOR_OPEN || sound == SoundEvents::IRON_DOOR_OPEN
             || sound == SoundEvents::WOODEN_TRAPDOOR_OPEN || sound == SoundEvents::IRON_TRAPDOOR_OPEN
             || sound == SoundEvents::FENCE_GATE_OPEN || sound == SoundEvents::WOODEN_DOOR_CLOSE
             || sound == SoundEvents::IRON_DOOR_CLOSE || sound == SoundEvents::WOODEN_TRAPDOOR_CLOSE
             || sound == SoundEvents::IRON_TRAPDOOR_CLOSE || sound == SoundEvents::FENCE_GATE_CLOSE)
        file = rand() % 2 == 0 ? &s_doorOpenSound : &s_doorCloseSound;
    return file != 0 && !file->empty() ? file : 0;
}

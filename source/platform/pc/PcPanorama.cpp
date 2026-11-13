#include "platform/pc/PcPanorama.hpp"

#include "platform/pc/PcButtonGlyphs.hpp"
#include "platform/pc/PcPaths.hpp"
#include "platform/pc/PcSettings.hpp"

#include "game/client/render/TexturePack.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/core/Minecraft.hpp"

#include <stdio.h>
#include <string.h>
#include <string>

namespace
{
const char* const sFiles[kPcPanoramaCount] = {
    "PreRelease",
    "TU1",
    "TU5",
    "TU9",
    "TU14",
    "TU19",
    "TU31",
    "TU46",
    "TU69"};
const wchar_t* const sNames[kPcPanoramaCount] = {
    L"Pre-Release",
    L"TU1",
    L"TU5",
    L"TU9",
    L"TU14",
    L"TU19",
    L"TU31",
    L"TU46",
    L"Latest"};

std::string PanoramaPath(int pano, bool night)
{
    return std::string("PC/Panoramas/") + sFiles[pano] + (night ? "_Night.png" : ".png");
}

bool DefaultPackSelected()
{
    Minecraft* mc = Minecraft::GetInstance();
    if (mc == 0 || mc->mTexturePackRepository == 0)
        return true;
    TexturePack* pk = mc->mTexturePackRepository->getSelected();
    return pk == 0 || pk == TexturePackRepository::defaultTexturePack;
}
}

const wchar_t* PcPanoramaName(int pano)
{
    return pano >= 0 && pano < kPcPanoramaCount ? sNames[pano] : sNames[kPcPanoramaDefault];
}

bool PcPanoramaAvailable(int pano)
{
    if (pano < 0 || pano >= kPcPanoramaCount)
        return false;
    if (pano == kPcPanoramaDefault)
        return true;
    FILE* fp = fopen(PcResolvePath(PanoramaPath(pano, false).c_str()).c_str(), "rb");
    if (fp == 0)
        return false;
    fclose(fp);
    return true;
}

int PcPanoramaTexture(const char* name, int textureId)
{
    const int pano = PcSettingsPanorama();
    if (pano < 0 || pano >= kPcPanoramaCount || pano == kPcPanoramaDefault)
        return textureId;
    const bool night = strcmp(name, "Panorama_Background_N") == 0;
    if (!night && strcmp(name, "Panorama_Background_S") != 0)
        return textureId;
    if (!DefaultPackSelected())
        return textureId;
    int res = night ? PcUiTexture(PanoramaPath(pano, true)) : -1;
    if (res < 0)
        res = PcUiTexture(PanoramaPath(pano, false));
    return res >= 0 ? res : textureId;
}

#include "platform/pc/PcTitleLogo.hpp"

#include "platform/pc/PcButtonGlyphs.hpp"
#include "platform/pc/PcPaths.hpp"
#include "platform/pc/PcSettings.hpp"

#include "game/client/render/TexturePack.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/Minecraft.hpp"

#include <string.h>
#include <string>

namespace
{
const char* const sFiles[kPcTitleLogoCount] = {
    "Default",
    "WiiU",
    "Xbox360",
    "XboxOne",
    "PS3",
    "PS4",
    "PSVita"};
const wchar_t* const sNames[kPcTitleLogoCount] = {
    L"Default",
    L"Wii U",
    L"Xbox 360",
    L"Xbox One",
    L"PS3",
    L"PS4",
    L"PS Vita"};
const wchar_t* const STOCK_LOGO_PACKS[] = {
    L"Candy Texture Pack",
    L"Cartoon Texture Pack",
    L"City Texture Pack",
    L"Fantasy Texture Pack",
    L"Halloween 2015",
    L"Natural Texture Pack",
    L"Pattern Texture Pack",
    L"Plastic Texture Pack",
    L"Steampunk Texture Pack",
    L"Super Cute Texture Pack"};

enum Variant
{
    eVariant_None,
    eVariant_Normal,
    eVariant_Festive
};

Variant SelectedVariant()
{
    Minecraft* mc = Minecraft::GetInstance();
    if (mc == 0 || mc->mTexturePackRepository == 0)
        return eVariant_Normal;
    TexturePack* pk = mc->mTexturePackRepository->getSelected();
    if (pk == 0 || pk == TexturePackRepository::defaultTexturePack)
        return eVariant_Normal;
    DLCPack* dlc = pk->getDLCPack();
    if (dlc == 0)
        return eVariant_None;
    const std::wstring name = dlc->getName();
    if (name == L"Festive")
        return eVariant_Festive;
    for (unsigned int i = 0; i < sizeof(STOCK_LOGO_PACKS) / sizeof(STOCK_LOGO_PACKS[0]); ++i)
    {
        if (name == STOCK_LOGO_PACKS[i])
            return eVariant_Normal;
    }
    return eVariant_None;
}
}

const wchar_t* PcTitleLogoName(int lg)
{
    return lg >= 0 && lg < kPcTitleLogoCount ? sNames[lg] : sNames[0];
}

int PcTitleLogoTexture(const char* name, int textureId)
{
    const int lg = PcSettingsTitleLogo();
    if (lg < 0 || lg >= kPcTitleLogoCount)
        return textureId;
    if (strcmp(name, "MenuTitle") != 0 && strcmp(name, "MenuTitleSmall") != 0)
        return textureId;
    const Variant variant = SelectedVariant();
    if (variant == eVariant_None)
        return textureId;
    const std::string base = std::string("PC/TitleLogos/") + sFiles[lg];
    int res = variant == eVariant_Festive ? PcUiTexture(base + "_Festive.png") : -1;
    if (res < 0)
        res = PcUiTexture(base + ".png");
    return res >= 0 ? res : textureId;
}

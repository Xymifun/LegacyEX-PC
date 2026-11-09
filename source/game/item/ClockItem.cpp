#include "game/item/ClockItem.hpp"

#include <cstring>

#include "game/core/IconRegister.hpp"
#include "game/core/Minecraft.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/ClockItem_ItemPropertyFunction.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

static const std::wstring CLOCK_ICON_NAMES[4] = {std::wstring(L"clockP0"), std::wstring(L"clockP1"),
                                                 std::wstring(L"clockP2"), std::wstring(L"clockP3")};

ClockItem::ClockItem() : Item()
{
    addProperty(ResourceLocation(L"time"), new ClockItem_ItemPropertyFunction());
    memset(mIcons, 0, sizeof(mIcons));
}

TextureAtlasSprite* ClockItem::getIcon(int auxValue)
{
    TextureAtlasSprite* icon = Item::getIcon(auxValue);
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mPlayer && auxValue == 0)
        icon = mIcons[static_cast<LocalPlayer*>(minecraft->mPlayer.get())->mUserIndex];
    return icon;
}

void ClockItem::registerIcons(IconRegister* iconRegister)
{
    Item::registerIcons(iconRegister);
    for (int i = 0; i < 4; ++i)
        mIcons[i] = iconRegister->registerIcon(CLOCK_ICON_NAMES[i]);
}

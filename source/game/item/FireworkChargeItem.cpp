#include "game/item/FireworkChargeItem.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/core/IconRegister.hpp"
#include "game/data/Tag.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/L10N.hpp"
#include "game/core/StringIds.hpp"
#include "game/item/DyeItem.hpp"
#include "game/item/FireworksItem.hpp"
#include "game/item/ItemInstance.hpp"

namespace
{
const int SHAPE_NAME_IDS[5] = {IDS_FIREWORKS_CHARGE_TYPE_0, IDS_FIREWORKS_CHARGE_TYPE_1, IDS_FIREWORKS_CHARGE_TYPE_2,
                               IDS_FIREWORKS_CHARGE_TYPE_3, IDS_FIREWORKS_CHARGE_TYPE_4};

const int COLOUR_NAME_IDS[16] = {
    IDS_FIREWORKS_CHARGE_BLACK,      IDS_FIREWORKS_CHARGE_RED,     IDS_FIREWORKS_CHARGE_GREEN,
    IDS_FIREWORKS_CHARGE_BROWN,      IDS_FIREWORKS_CHARGE_BLUE,    IDS_FIREWORKS_CHARGE_PURPLE,
    IDS_FIREWORKS_CHARGE_CYAN,       IDS_FIREWORKS_CHARGE_SILVER,  IDS_FIREWORKS_CHARGE_GRAY,
    IDS_FIREWORKS_CHARGE_PINK,       IDS_FIREWORKS_CHARGE_LIME,    IDS_FIREWORKS_CHARGE_YELLOW,
    IDS_FIREWORKS_CHARGE_LIGHT_BLUE, IDS_FIREWORKS_CHARGE_MAGENTA, IDS_FIREWORKS_CHARGE_ORANGE,
    IDS_FIREWORKS_CHARGE_WHITE};
}

FireworkChargeItem::FireworkChargeItem() : Item()
{
}

Tag* FireworkChargeItem::getExplosionTagField(not_null_ptr<ItemInstance> item, const std::wstring& name)
{
    if (item->hasTag())
    {
        CompoundTag* explosion = item->getTag()->getCompound(FireworksItem::TAG_EXPLOSION);
        if (explosion != NULL)
        {
            return explosion->get(name);
        }
    }
    return NULL;
}

void FireworkChargeItem::appendHoverText(CompoundTag* tag, std::vector<HtmlString>* lines)
{
    unsigned char shape = tag->getByte(FireworksItem::TAG_E_TYPE);
    if (shape <= 4)
    {
        int nameId = SHAPE_NAME_IDS[shape];
        lines->push_back(HtmlString(std::wstring(L10N::GetString(nameId)), eMinecraftColour_NOTSET, false, false, 0));
    }
    else
    {
        lines->push_back(HtmlString(std::wstring(L10N::GetString(IDS_FIREWORKS_CHARGE_TYPE)), eMinecraftColour_NOTSET,
                                    false, false, 0));
    }

    arrayWithLength<int> colours = tag->getIntArray(FireworksItem::TAG_E_COLORS);
    if (colours.length > 0)
    {
        bool first = true;
        std::wstring text = L"";
        for (unsigned int i = 0; i < colours.length; ++i)
        {
            int colour = colours[i];
            if (!first)
            {
                text += L",\n";
            }
            first = false;
            bool found = false;
            for (int dye = 0; dye < 16; ++dye)
            {
                if (colour == DyeItem::DYE_COLORS[dye])
                {
                    found = true;
                    text += L10N::GetString(COLOUR_NAME_IDS[dye]);
                    break;
                }
            }
            if (!found)
            {
                text += L10N::GetString(IDS_FIREWORKS_CHARGE_CUSTOM);
            }
        }
        lines->push_back(HtmlString(text, eMinecraftColour_NOTSET, false, false, 0));
    }

    arrayWithLength<int> fadeColours = tag->getIntArray(FireworksItem::TAG_E_FADECOLORS);
    if (fadeColours.length > 0)
    {
        bool first = true;
        std::wstring text = std::wstring(L10N::GetString(IDS_FIREWORKS_CHARGE_FADE_TO)) + L" ";
        for (unsigned int i = 0; i < fadeColours.length; ++i)
        {
            int colour = fadeColours[i];
            if (!first)
            {
                text += L",\n";
            }
            first = false;
            bool found = false;
            for (int dye = 0; dye < 16; ++dye)
            {
                if (colour == DyeItem::DYE_COLORS[dye])
                {
                    found = true;
                    text += L10N::GetString(COLOUR_NAME_IDS[dye]);
                    break;
                }
            }
            if (!found)
            {
                text += L10N::GetString(IDS_FIREWORKS_CHARGE_CUSTOM);
            }
        }
        lines->push_back(HtmlString(text, eMinecraftColour_NOTSET, false, false, 0));
    }

    if (tag->getBoolean(FireworksItem::TAG_E_TRAIL))
    {
        lines->push_back(HtmlString(std::wstring(L10N::GetString(IDS_FIREWORKS_CHARGE_TRAIL)), eMinecraftColour_NOTSET,
                                    false, false, 0));
    }

    if (tag->getBoolean(FireworksItem::TAG_E_FLICKER))
    {
        lines->push_back(HtmlString(std::wstring(L10N::GetString(IDS_FIREWORKS_CHARGE_FLICKER)),
                                    eMinecraftColour_NOTSET, false, false, 0));
    }
}

void FireworkChargeItem::appendHoverText(not_null_ptr<ItemInstance> item, boost::shared_ptr<Player> player,
                                         std::vector<HtmlString>* lines, bool advanced)
{
    if (item->hasTag())
    {
        CompoundTag* explosion = item->getTag()->getCompound(FireworksItem::TAG_EXPLOSION);
        if (explosion != NULL)
        {
            appendHoverText(explosion, lines);
        }
    }
}

TextureAtlasSprite* FireworkChargeItem::getLayerIcon(int auxValue, int layer, not_null_ptr<ItemInstance> item)
{
    if (layer > 0)
    {
        return mOverlayIcon;
    }
    return Item::getLayerIcon(auxValue, layer, item);
}

bool FireworkChargeItem::hasMultipleSpriteLayers()
{
    return true;
}

void FireworkChargeItem::registerIcons(IconRegister* iconRegister)
{
    Item::registerIcons(iconRegister);
    mOverlayIcon = iconRegister->registerIcon(getIconName() + L"_overlay");
}

#include "game/entity/mob/HorseArmorItem.hpp"
#include "game/core/Color.hpp"
#include "game/item/Items.hpp"

#include "game/core/IconRegister.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/item/ItemInstance.hpp"

static const std::wstring HORSE_ARMOR_BASE_ICON = L"leather_horse_armor_base";
const std::wstring HorseArmorItem::HORSE_ARMOR_DETAIL_ICON = L"leather_horse_armor_detail";
static const std::wstring HORSE_ARMOR_COLOR = L"customColor";

HorseArmorItem::HorseArmorItem(int tier, int protection, int texture, const wchar_t* hash)
    : Item(),
      mTier(tier),
      mProtection(protection)
{
    mTextures.push_back(texture);
    mHash.assign(hash);
    mIcon = NULL;
    mArmorIcon = NULL;
}

HorseArmorItem::HorseArmorItem(int tier, int protection, const int* textures, int textureCount, const wchar_t* hash)
    : Item(),
      mTier(tier),
      mProtection(protection)
{
    for (int i = 0; i < textureCount; ++i)
    {
        mTextures.push_back(textures[i]);
    }
    mHash.assign(hash);
    mIcon = NULL;
    mArmorIcon = NULL;
}

HorseArmorItem::HorseArmorItem(int tier, int protection) : Item(), mTier(tier), mProtection(protection)
{
    mTextures.push_back(-1);
    mHash.assign(L"");
    mIcon = NULL;
    mArmorIcon = NULL;
}

bool HorseArmorItem::isHorseArmor(Item* item)
{
    return item == Items::leatherHorseArmor || item == Items::ironHorseArmor || item == Items::goldenHorseArmor
           || item == Items::diamondHorseArmor;
}

bool HorseArmorItem::isDyeable() const
{
    return getTier() == 1;
}

int HorseArmorItem::getTier() const
{
    return mTier;
}

std::wstring HorseArmorItem::getHash() const
{
    return mHash;
}

int HorseArmorItem::getProtection() const
{
    return mProtection;
}

int HorseArmorItem::getTexture(int index) const
{
    return mTextures.at(index);
}

int HorseArmorItem::getTextureCount() const
{
    return (int)mTextures.size();
}

TextureAtlasSprite* HorseArmorItem::getIcon(int auxValue)
{
    return mIcon;
}

TextureAtlasSprite* HorseArmorItem::getLayerIcon(int auxValue, int layer, not_null_ptr<ItemInstance> itemInstance)
{
    if (getTier() == 1 && layer == 0)
    {
        return mArmorIcon;
    }
    return Item::getLayerIcon(auxValue, layer, itemInstance);
}

bool HorseArmorItem::hasMultipleSpriteLayers()
{
    return true;
}

void HorseArmorItem::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(getIconName());
    if (getTier() == 1)
    {
        mArmorIcon = iconRegister->registerIcon(HORSE_ARMOR_BASE_ICON);
    }
}

bool HorseArmorItem::hasCustomColor(const not_null_ptr<ItemInstance>& itemInstance)
{
    if (itemInstance->hasTag())
    {
        return itemInstance->getTag()->contains(HORSE_ARMOR_COLOR);
    }
    return false;
}

int HorseArmorItem::getDyedColor(not_null_ptr<ItemInstance> itemInstance) const
{
    if (getTier() != 1)
    {
        return Color(255, 255, 255).getRGB();
    }
    if (itemInstance->hasTag())
    {
        CompoundTag* tag = itemInstance->getTag();
        if (tag->contains(HORSE_ARMOR_COLOR))
        {
            return tag->getInt(HORSE_ARMOR_COLOR);
        }
    }
    return DEFAULT_LEATHER_COLOR;
}

int HorseArmorItem::getColor(not_null_ptr<ItemInstance> itemInstance, int layer)
{
    if (layer != 0)
    {
        return Color(255, 255, 255).getRGB();
    }
    return getDyedColor(itemInstance);
}

void HorseArmorItem::clearColor(const not_null_ptr<ItemInstance>& itemInstance)
{
    if (itemInstance->hasTag())
    {
        itemInstance->getTag()->remove(HORSE_ARMOR_COLOR);
    }
}

void HorseArmorItem::setColor(const not_null_ptr<ItemInstance>& itemInstance, int color)
{
    if (!itemInstance->hasTag())
    {
        itemInstance->setTag(new CompoundTag());
    }
    itemInstance->getTag()->putInt(HORSE_ARMOR_COLOR, color);
}

std::wstring HorseArmorItem::getTexture() const
{
    return mHash;
}

bool HorseArmorItem::hasMultipleTextures() const
{
    return mTextures.size() > 1;
}

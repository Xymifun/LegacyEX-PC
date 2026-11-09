#ifndef GAME_ENTITY_MOB_HORSEARMORITEM_HPP
#define GAME_ENTITY_MOB_HORSEARMORITEM_HPP

#include <string>
#include <vector>

#include "game/item/Item.hpp"
#include "game/item/not_null_ptr.hpp"

class CompoundTag;
class IconRegister;
class TextureAtlasSprite;

class HorseArmorItem : public Item
{
public:
    static const int DEFAULT_LEATHER_COLOR = 0xA06540;
    static const std::wstring HORSE_ARMOR_DETAIL_ICON;

    HorseArmorItem(int tier, int protection, int texture, const wchar_t* hash);
    HorseArmorItem(int tier, int protection, const int* textures, int textureCount, const wchar_t* hash);
    HorseArmorItem(int tier, int protection);

    static bool isHorseArmor(Item* item);
    int getTier() const;
    std::wstring getHash() const;
    int getProtection() const;
    int getTexture(int index) const;
    int getTextureCount() const;
    virtual TextureAtlasSprite* getIcon(int auxValue);
    virtual TextureAtlasSprite* getLayerIcon(int auxValue, int layer, not_null_ptr<ItemInstance> itemInstance);
    virtual bool hasMultipleSpriteLayers();
    virtual void registerIcons(IconRegister* iconRegister);
    bool hasCustomColor(const not_null_ptr<ItemInstance>& itemInstance);
    int getDyedColor(not_null_ptr<ItemInstance> itemInstance) const;
    virtual int getColor(not_null_ptr<ItemInstance> itemInstance, int layer);
    void clearColor(const not_null_ptr<ItemInstance>& itemInstance);
    void setColor(const not_null_ptr<ItemInstance>& itemInstance, int color);
    bool isDyeable() const;
    std::wstring getTexture() const;
    bool hasMultipleTextures() const;

private:
    TextureAtlasSprite* mArmorIcon;
    int mTier;
    std::vector<int> mTextures;
    std::wstring mHash;
    int mProtection;
};

#endif

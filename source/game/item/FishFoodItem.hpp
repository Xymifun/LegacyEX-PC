#ifndef GAME_ITEM_FISHFOODITEM_HPP
#define GAME_ITEM_FISHFOODITEM_HPP

#include <string>

#include "game/item/FoodItem.hpp"

class IconRegister;
class TextureAtlasSprite;

class FishFoodItem : public FoodItem
{
public:
    class Type
    {
    public:
        Type(int id, unsigned int name, unsigned int description, unsigned int cookedName,
             unsigned int cookedDescription, int nutrition, float saturation, int cookedNutrition,
             float cookedSaturation, const std::wstring& icon, const std::wstring& cookedIcon);
        Type(int id, unsigned int name, unsigned int description, int nutrition, float saturation,
             const std::wstring& icon);

        static Type* getById(int id);

        static Type* COD;
        static Type* SALMON;
        static Type* CLOWNFISH;
        static Type* PUFFERFISH;

        static Type* getByItem(not_null_ptr<ItemInstance> itemInstance);

        unsigned char canBeCooked() const;
        unsigned int getCookedNutrition() const;
        unsigned int getRawNutrition() const;
        float getCookedSaturation() const;
        float getRawSaturation() const;
        int getName(bool cooked) const;
        int getDescription(bool cooked) const;
        TextureAtlasSprite* getIcon(bool cooked);
        void registerIcons(IconRegister* iconRegister);
        unsigned int getId() const;
        unsigned int getData() const;

    private:
        unsigned int mId;
        unsigned int mName;
        int mCookedName;
        unsigned int mRawNutrition;
        float mRawSaturation;
        unsigned int mCookedNutrition;
        float mCookedSaturation;
        unsigned char mCanBeCooked;
        std::wstring mIcon;
        std::wstring mCookedIcon;
        TextureAtlasSprite* mRawIcon;
        TextureAtlasSprite* mCookedIconValue;
        unsigned int mDescription;
        int mCookedDescription;
    };

    FishFoodItem(bool cooked);
    virtual ~FishFoodItem();
    virtual int getNutrition(not_null_ptr<ItemInstance> itemInstance);
    virtual float getSaturationModifier(not_null_ptr<ItemInstance> itemInstance);
    virtual void addEatEffect(not_null_ptr<ItemInstance> itemInstance, Level* level, boost::shared_ptr<Player> player);
    virtual int getDescriptionId(not_null_ptr<ItemInstance> itemInstance);
    virtual int getUseDescriptionId(not_null_ptr<ItemInstance> itemInstance);
    virtual TextureAtlasSprite* getIcon(int auxValue);
    virtual void registerIcons(IconRegister* iconRegister);

private:
    bool mCooked;
};

extern FishFoodItem::Type* gFishFoodTypes[4];

#endif

#include "game/item/FishFoodItem.hpp"

#include "game/core/FoodConstants.hpp"
#include "game/core/StringIds.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/IconRegister.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/world/generation/Player.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"

FishFoodItem::Type* FishFoodItem::Type::COD = new FishFoodItem::Type(
    0, IDS_ITEM_FISH_COD_RAW, IDS_DESC_FISH_COD_RAW, IDS_ITEM_FISH_COD_COOKED, IDS_DESC_FISH_COD_COOKED, 2,
    FoodConstants::FOOD_SATURATION_POOR, 5, FoodConstants::FOOD_SATURATION_NORMAL, L"fishRaw", L"fishCooked");
FishFoodItem::Type* FishFoodItem::Type::SALMON = new FishFoodItem::Type(
    1, IDS_ITEM_FISH_SALMON_RAW, IDS_DESC_FISH_SALMON_RAW, IDS_ITEM_FISH_SALMON_COOKED, IDS_DESC_FISH_SALMON_COOKED, 2,
    FoodConstants::FOOD_SATURATION_POOR, 6, FoodConstants::FOOD_SATURATION_GOOD, L"fish_salmon_raw",
    L"fish_salmon_cooked");
FishFoodItem::Type* FishFoodItem::Type::CLOWNFISH = new FishFoodItem::Type(
    2, 0x0B072C6C, 0xCC0E58F0, 1, FoodConstants::FOOD_SATURATION_POOR, L"fish_clownfish_raw");
FishFoodItem::Type* FishFoodItem::Type::PUFFERFISH = new FishFoodItem::Type(
    3, IDS_ITEM_FISH_PUFFERFISH_RAW, IDS_DESC_FISH_PUFFERFISH_RAW, 1, FoodConstants::FOOD_SATURATION_POOR,
    L"fish_pufferfish_raw");
FishFoodItem::Type* gFishFoodTypes[4] = {FishFoodItem::Type::COD, FishFoodItem::Type::SALMON,
                                         FishFoodItem::Type::CLOWNFISH, FishFoodItem::Type::PUFFERFISH};

FishFoodItem::FishFoodItem(bool cooked) : FoodItem(0, 0.0f, false), mCooked(cooked)
{
}

FishFoodItem::~FishFoodItem()
{
}

FishFoodItem::Type::Type(int id, unsigned int name, unsigned int description, unsigned int cookedName,
                         unsigned int cookedDescription, int nutrition, float saturation, int cookedNutrition,
                         float cookedSaturation, const std::wstring& icon, const std::wstring& cookedIcon)
    : mId(id),
      mName(name),
      mCookedName(cookedName),
      mRawNutrition(nutrition),
      mRawSaturation(saturation),
      mCookedNutrition(cookedNutrition),
      mCookedSaturation(cookedSaturation),
      mCanBeCooked(1),
      mIcon(icon),
      mCookedIcon(cookedIcon),
      mRawIcon(0),
      mCookedIconValue(0),
      mDescription(description),
      mCookedDescription(cookedDescription)
{
}

FishFoodItem::Type::Type(int id, unsigned int name, unsigned int description, int nutrition, float saturation,
                         const std::wstring& icon)
{
    mId = id;
    mName = name;
    mCookedName = -1;
    mRawNutrition = nutrition;
    mRawSaturation = saturation;
    mCookedNutrition = 0;
    mCookedSaturation = 0.0f;
    mCanBeCooked = 0;
    mIcon = icon;
    mCookedIcon = L"";
    mDescription = description;
    mCookedDescription = -1;
    mRawIcon = 0;
    mCookedIconValue = 0;
}

FishFoodItem::Type* FishFoodItem::Type::getById(int id)
{
    if (id < 0 || id > 3)
    {
        return NULL;
    }
    return gFishFoodTypes[id];
}

FishFoodItem::Type* FishFoodItem::Type::getByItem(not_null_ptr<ItemInstance> itemInstance)
{
    if (dynamic_cast<FishFoodItem*>(itemInstance->getItem()) != NULL)
    {
        return getById(itemInstance->getAuxValue());
    }
    return COD;
}

unsigned char FishFoodItem::Type::canBeCooked() const
{
    return mCanBeCooked;
}

unsigned int FishFoodItem::Type::getCookedNutrition() const
{
    return mCookedNutrition;
}

unsigned int FishFoodItem::Type::getRawNutrition() const
{
    return mRawNutrition;
}

float FishFoodItem::Type::getCookedSaturation() const
{
    return mCookedSaturation;
}

float FishFoodItem::Type::getRawSaturation() const
{
    return mRawSaturation;
}

int FishFoodItem::Type::getName(bool cooked) const
{
    return cooked && mCookedName != -1 ? mCookedName : mName;
}

int FishFoodItem::Type::getDescription(bool cooked) const
{
    return cooked && mCookedDescription != -1 ? mCookedDescription : mDescription;
}

TextureAtlasSprite* FishFoodItem::Type::getIcon(bool cooked)
{
    if (cooked && mCookedIconValue != 0)
    {
        return mCookedIconValue;
    }
    return mRawIcon;
}

void FishFoodItem::Type::registerIcons(IconRegister* iconRegister)
{
    mRawIcon = iconRegister->registerIcon(mIcon);
    if (!mCookedIcon.empty())
    {
        mCookedIconValue = iconRegister->registerIcon(mCookedIcon);
    }
}

unsigned int FishFoodItem::Type::getId() const
{
    return mId;
}

unsigned int FishFoodItem::Type::getData() const
{
    return mId;
}

int FishFoodItem::getNutrition(not_null_ptr<ItemInstance> itemInstance)
{
    Type* type = Type::getByItem(itemInstance);
    if (mCooked && type->canBeCooked())
    {
        return (int)type->getCookedNutrition();
    }
    return (int)type->getRawNutrition();
}

float FishFoodItem::getSaturationModifier(not_null_ptr<ItemInstance> itemInstance)
{
    Type* type = Type::getByItem(itemInstance);
    if (mCooked && type->canBeCooked())
    {
        return type->getCookedSaturation();
    }
    return type->getRawSaturation();
}

void FishFoodItem::addEatEffect(not_null_ptr<ItemInstance> itemInstance, Level* level, boost::shared_ptr<Player> player)
{
    Type* type = Type::getByItem(itemInstance);
    if (type == Type::PUFFERFISH)
    {
        player->addEffect(new MobEffectInstance(MobEffects::POISON, 1200, 3), boost::shared_ptr<Entity>());
        player->addEffect(new MobEffectInstance(MobEffects::HUNGER, 300, 2), boost::shared_ptr<Entity>());
        player->addEffect(new MobEffectInstance(MobEffects::CONFUSION, 300, 1), boost::shared_ptr<Entity>());
    }
    FoodItem::addEatEffect(itemInstance, level, player);
}

int FishFoodItem::getDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    FishFoodItem* item = static_cast<FishFoodItem*>(itemInstance->getItem());
    return Type::getByItem(itemInstance)->getName(item->mCooked);
}

int FishFoodItem::getUseDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    FishFoodItem* item = static_cast<FishFoodItem*>(itemInstance->getItem());
    return Type::getByItem(itemInstance)->getDescription(item->mCooked);
}

TextureAtlasSprite* FishFoodItem::getIcon(int auxValue)
{
    return Type::getById(auxValue)->getIcon(mCooked);
}

void FishFoodItem::registerIcons(IconRegister* iconRegister)
{
    for (int i = 0; i < 4; i++)
    {
        gFishFoodTypes[i]->registerIcons(iconRegister);
    }
}

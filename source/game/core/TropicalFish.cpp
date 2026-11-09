#include "game/core/TropicalFish.hpp"
#include "game/core/EntityType.hpp"

#include <typeinfo>

#include "game/core/AttributeInstance.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/world/generation/Player.hpp"
#include "game/util/Random.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/AvoidEntityGoal.hpp"
#include "game/entity/ai/RandomSwimmingGoal.hpp"
#include "game/entity/ai/SwimIdleGoal.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/world/Level.hpp"
#include "game/core/PIXNamedEvents.hpp"


const EntityDataAccessor<int>* TropicalFish::DATA_VARIANT = NULL;
const EntityDataAccessor<int>* TropicalFish::DATA_COLOR = NULL;

static const int TROPICAL_FISH_VARIANTS[22][4] = {
    {1, 1, 1, 7},  {1, 0, 7, 7}, {0, 1, 7, 3}, {0, 4, 0, 7},  {0, 1, 11, 7}, {0, 0, 1, 0}, {0, 5, 6, 3},  {1, 3, 10, 4},
    {1, 5, 0, 14}, {0, 5, 0, 4}, {1, 2, 0, 7}, {1, 5, 0, 1},  {0, 3, 9, 6},  {0, 4, 5, 3}, {1, 4, 14, 0}, {0, 2, 7, 14},
    {1, 3, 14, 0}, {1, 0, 0, 4}, {0, 1, 0, 0}, {0, 0, 14, 0}, {1, 1, 4, 4},  {0, 3, 0, 7}};
static int TROPICAL_FISH_BASE_TEXTURES[2] = {0xD6, 0xDD};
static int TROPICAL_FISH_PATTERN_A_TEXTURES[6] = {0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC};
static int TROPICAL_FISH_PATTERN_B_TEXTURES[6] = {0xDE, 0xDF, 0xE0, 0xE1, 0xE2, 0xE3};
static const std::wstring TROPICAL_FISH_BASE_HASHES[2] = {L"fba", L"fbb"};
static const std::wstring TROPICAL_FISH_PATTERN_A_HASHES[6] = {L"pa1", L"pa2", L"pa3", L"pa4", L"pa5", L"pa6"};
static const std::wstring TROPICAL_FISH_PATTERN_B_HASHES[6] = {L"pb1", L"pb2", L"pb3", L"pb4", L"pb5", L"pb6"};

TropicalFish::TropicalFish(Level* level) : Fish(level, true)
{
    fjDerivedCtorCalls();
    mLayeredTextureHashName = L"";
    mLayeredTextureLayers = arrayWithLength<int>(2, true);
    mLayeredTextureLayers.data[0] = -1;
    mLayeredTextureLayers.data[1] = -1;
    mSwimAmplitude = 0.01f;
    mSwimFrequency = 0.02f;
    mWaterValueC = 12.0f;
    setSize(0.4f, 0.4f);
}

TropicalFish::~TropicalFish()
{
    delete[] mLayeredTextureLayers.data;
}

Entity* TropicalFish::create(Level* level)
{
    return new TropicalFish(level);
}

void TropicalFish::staticCtor()
{
    DATA_VARIANT = SynchedEntityData::defineId(EntityType::TROPICAL_FISH, EntityType::WATER_ANIMAL,
                                               EntityDataSerializers::INT);
    DATA_COLOR = SynchedEntityData::defineId(EntityType::TROPICAL_FISH, EntityType::WATER_ANIMAL,
                                             EntityDataSerializers::INT);
}

void TropicalFish::clearLayeredTextureInfo()
{
    mLayeredTextureHashName.erase(mLayeredTextureHashName.begin(), mLayeredTextureHashName.end());
}

void TropicalFish::tick()
{
    PIXBeginNamedEvent(0.0f, "TropicalFish tick");
    Mob::tick();
    if (mLevel->mIsClientSide && mEntityData->isDirty())
    {
        mEntityData->clearDirty();
        clearLayeredTextureInfo();
    }
    PIXEndNamedEvent();
}

int TropicalFish::getVariant()
{
    return mEntityData->get(DATA_VARIANT);
}
int TropicalFish::getBaseVariant()
{
    return mEntityData->get(DATA_VARIANT) & 1;
}
int TropicalFish::getMarkVariant()
{
    return (mEntityData->get(DATA_VARIANT) >> 8 & 0xff) % 6;
}

void TropicalFish::setVariant(int variant)
{
    mEntityData->set(DATA_VARIANT, variant, true);
    clearLayeredTextureInfo();
}

int TropicalFish::getColor()
{
    return mEntityData->get(DATA_COLOR);
}
int TropicalFish::getColor1()
{
    return (mEntityData->get(DATA_COLOR) & 0xff) % 15;
}
int TropicalFish::getColor2()
{
    return (mEntityData->get(DATA_COLOR) >> 8 & 0xff) % 15;
}
void TropicalFish::setColor(int color)
{
    mEntityData->set(DATA_COLOR, color, true);
}

void TropicalFish::rebuildLayeredTextureInfo()
{
    mLayeredTextureHashName = L"fish/";

    for (unsigned int i = 0; i < mLayeredTextureLayers.length; i++)
    {
        mLayeredTextureLayers.data[i] = -1;
    }

    int baseVariant = getBaseVariant();
    int markVariant = getMarkVariant();
    mLayeredTextureLayers.data[0] = TROPICAL_FISH_BASE_TEXTURES[baseVariant];
    if (baseVariant == 0)
    {
        mLayeredTextureLayers.data[1] = TROPICAL_FISH_PATTERN_A_TEXTURES[markVariant];
    }
    else
    {
        mLayeredTextureLayers.data[1] = TROPICAL_FISH_PATTERN_B_TEXTURES[markVariant];
    }

    mLayeredTextureHashName = L"fish/" + TROPICAL_FISH_BASE_HASHES[baseVariant]
                              + (baseVariant == 0 ? TROPICAL_FISH_PATTERN_A_HASHES[markVariant]
                                                  : TROPICAL_FISH_PATTERN_B_HASHES[markVariant]);
}

std::wstring TropicalFish::getLayeredTextureHashName()
{
    if (mLayeredTextureHashName.empty())
        rebuildLayeredTextureInfo();
    return mLayeredTextureHashName;
}

arrayWithLength<int> TropicalFish::getLayeredTextureLayers()
{
    if (mLayeredTextureHashName.empty())
        rebuildLayeredTextureInfo();
    return mLayeredTextureLayers;
}

void TropicalFish::registerGoals()
{
    mGoalSelector.addGoal(3, new SwimIdleGoal(this), true);
    mGoalSelector.addGoal(4, new RandomSwimmingGoal(this, 1.0, 10, 16, 4), true);
    mGoalSelector.addGoal(2, new AvoidEntityGoal(this, typeid(Player), 6.0f, 0.3, 0.4), true);
}

void TropicalFish::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(6.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(1.2);
}

void TropicalFish::defineSynchedData()
{
    Mob::defineSynchedData();
    mEntityData->createDateItem(DATA_VARIANT, 0);
    mEntityData->createDateItem(DATA_COLOR, 0);
}

void TropicalFish::addAdditonalSaveData(CompoundTag* tag)
{
    Mob::addAdditonalSaveData(tag);
    tag->putInt(L"Variant", getVariant());
    tag->putInt(L"Color", getColor());
}

void TropicalFish::readAdditionalSaveData(CompoundTag* tag)
{
    Mob::readAdditionalSaveData(tag);
    setVariant(tag->getInt(L"Variant"));
    setColor(tag->getInt(L"Color"));
}

MobGroupData* TropicalFish::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                             const std::wstring& name)
{
    mRandom = new Random();
    int baseVariant;
    int markVariant;
    int color1;
    int color2;
    if (dynamic_cast<TropicalFishGroupData*>(data) != NULL)
    {
        TropicalFishGroupData* group = static_cast<TropicalFishGroupData*>(data);
        baseVariant = group->mBaseVariant;
        markVariant = group->mMarkVariant;
        color1 = group->mColor1;
        color2 = group->mColor2;
    }
    else if (mRandom->nextFloat() < 0.9)
    {
        int index = mRandom->nextInt(22);
        baseVariant = TROPICAL_FISH_VARIANTS[index][0];
        markVariant = TROPICAL_FISH_VARIANTS[index][1];
        color1 = TROPICAL_FISH_VARIANTS[index][2];
        color2 = TROPICAL_FISH_VARIANTS[index][3];
        data = new TropicalFishGroupData(baseVariant, markVariant, color1, color2);
    }
    else
    {
        baseVariant = mRandom->nextInt(2);
        markVariant = mRandom->nextInt(6);
        color1 = mRandom->nextInt(15);
        color2 = mRandom->nextInt(15);
    }
    setVariant(baseVariant | (markVariant << 8));
    setColor(color1 | (color2 << 8));
    return Mob::finalizeMobSpawn(difficulty, data, std::wstring(L""));
}

ResourceLocation* TropicalFish::getDefaultLootTable()
{
    return &BuiltInLootTables::TROPICALFISH;
}
unsigned long long TropicalFish::GetType()
{
    return EntityType::TROPICAL_FISH;
}

TropicalFish::TropicalFishGroupData::TropicalFishGroupData(int baseVariant, int markVariant, int color1, int color2)
    : mBaseVariant(baseVariant),
      mMarkVariant(markVariant),
      mColor1(color1),
      mColor2(color2)
{
}

TropicalFish::TropicalFishGroupData::~TropicalFishGroupData()
{
}

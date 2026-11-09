#include "game/core/Sheep.hpp"

#include <algorithm>

#include "game/entity/mob/EntityEvent.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/DyeColor.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/Mth.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/AgableMob.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/entity/ai/BreedGoal.hpp"
#include "game/entity/ai/FloatGoal.hpp"
#include "game/entity/ai/FollowParentGoal.hpp"
#include "game/entity/ai/GoalSelector.hpp"
#include "game/entity/ai/PanicGoal.hpp"
#include "game/entity/ai/RandomLookAroundGoal.hpp"
#include "game/entity/ai/TemptGoal.hpp"
#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Recipes.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/world/block/EatBlockGoal.hpp"
#include "game/world/generation/LookAtPlayerGoal.hpp"
#include "game/world/generation/Player.hpp"
#include "game/item/ItemId.hpp"

static const EntityDataAccessor<unsigned char>* gSheepSheared = NULL;
static boost::unordered_map<const DyeColor*, arrayWithLength<const float> > gSheepColors;

Sheep::Sheep(Level* level) : Animal(level)
{
    fjDerivedCtorCalls();
    setSize(0.9f, 1.3f);
    mEatAnimationTick = 0;
    mContainer = new SheepContainer();
    mCraftingContainer = boost::shared_ptr<CraftingContainer>(new CraftingContainer(mContainer, 2, 1));
    mCraftingContainer->setItem(0, not_null_ptr<ItemInstance>(new ItemInstance(Items::dye, 1, 0)));
    mCraftingContainer->setItem(1, not_null_ptr<ItemInstance>(new ItemInstance(Items::dye, 1, 0)));
}

Sheep::~Sheep()
{
    delete mContainer;
}

Entity* Sheep::create(Level* level)
{
    return new Sheep(level);
}

arrayWithLength<const float> Sheep::getColorArray(const DyeColor* color)
{
    return gSheepColors[color];
}

arrayWithLength<const float> Sheep::createSheepColor(const DyeColor* color)
{
    arrayWithLength<const float> diffuseColors = color->getTextureDiffuseColors();

    float* values = new float[3];
    values[0] = diffuseColors.data[0] * 0.75f;
    values[1] = diffuseColors.data[1] * 0.75f;
    values[2] = diffuseColors.data[2] * 0.75f;

    arrayWithLength<const float> colors;
    colors.data = values;
    colors.length = 3;
    return colors;
}

void Sheep::staticCtor()
{
    gSheepSheared = SynchedEntityData::defineId(EntityType::SHEEP, EntityType::ANIMAL, EntityDataSerializers::BYTE);

    for (std::vector<const DyeColor*>::const_iterator it = DyeColor::COLORS.begin(); it != DyeColor::COLORS.end(); ++it)
    {
        const DyeColor* color = *it;
        gSheepColors[color] = createSheepColor(color);
    }

    float* white = new float[3];
    white[0] = 0.9019608f;
    white[1] = 0.9019608f;
    white[2] = 0.9019608f;
    gSheepColors[DyeColor::WHITE] = arrayWithLength<const float>(white, 3);
}

void Sheep::registerGoals()
{
    mEatBlockGoal = new EatBlockGoal(this);

    mGoalSelector.addGoal(0, new FloatGoal(this), true);
    mGoalSelector.addGoal(1, new PanicGoal(this, 1.25), true);
    mGoalSelector.addGoal(2, new BreedGoal(this, 1.0), true);

    static Item* temptItems[] = {Items::wheat};
    mGoalSelector.addGoal(
        3, new TemptGoal(this, 1.1, arrayWithLength<Item*>(temptItems, 1), false, arrayWithLength<int>()), true);

    mGoalSelector.addGoal(4, new FollowParentGoal(this, 1.1), true);
    mGoalSelector.addGoal(5, mEatBlockGoal, true);
    mGoalSelector.addGoal(6, new WaterAvoidingRandomStrollGoal(this, 1.0), true);
    mGoalSelector.addGoal(7, new LookAtPlayerGoal(this, typeid(Player), 6.0f), true);
    mGoalSelector.addGoal(8, new RandomLookAroundGoal(this), true);
}

void Sheep::customServerAiStep()
{
    mEatAnimationTick = static_cast<EatBlockGoal*>(mEatBlockGoal)->getEatAnimationTick();
    Animal::customServerAiStep();
}

void Sheep::aiStep()
{
    if (mLevel->mIsClientSide)
    {
        mEatAnimationTick = std::max(0, mEatAnimationTick - 1);
    }
    Animal::aiStep();
}

void Sheep::registerAttributes()
{
    Mob::registerAttributes();
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(8.0);
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(0.23000000417232513);
}

void Sheep::defineSynchedData()
{
    Animal::defineSynchedData();
    mEntityData->createDateItem(gSheepSheared, static_cast<unsigned char>(0));
}

ResourceLocation* Sheep::getDefaultLootTable()
{
    return &BuiltInLootTables::SHEEP;
}

void Sheep::handleEntityEvent(unsigned char event, int data)
{
    if (event == EntityEvent::EAT_GRASS)
    {
        mEatAnimationTick = 40;
        return;
    }

    Animal::handleEntityEvent(event, data);
}

float Sheep::getHeadEatPositionScale(float partialTick)
{
    if (mEatAnimationTick < 1)
    {
        return 0.0f;
    }
    if (mEatAnimationTick >= 4 && mEatAnimationTick <= 36)
    {
        return 1.0f;
    }
    if (mEatAnimationTick < 4)
    {
        return (mEatAnimationTick - partialTick) * 0.25f;
    }
    return -((mEatAnimationTick - 40 - partialTick) * 0.25f);
}

float Sheep::getHeadEatAngleScale(float partialTick)
{
    if (mEatAnimationTick > 4 && mEatAnimationTick <= 36)
    {
        return 0.62831855f + 0.21991149f * Mth::sin((mEatAnimationTick - 4 - partialTick) * 0.03125f * 28.7f);
    }
    float scale = 0.62831855f;
    if (mEatAnimationTick <= 0)
    {
        scale = mXRot * Mth::DEG_TO_RAD;
    }
    return scale;
}

bool Sheep::isSheared()
{
    return (mEntityData->get(gSheepSheared) & 0x10) != 0;
}

void Sheep::setSheared(bool sheared)
{
    unsigned char value = mEntityData->get(gSheepSheared);
    if (sheared)
    {
        mEntityData->set(gSheepSheared, static_cast<unsigned char>(value | 0x10), true);
    }
    else
    {
        mEntityData->set(gSheepSheared, static_cast<unsigned char>(value & 0xef), true);
    }
}

const DyeColor* Sheep::getColor()
{
    return DyeColor::byBlockData(mEntityData->get(gSheepSheared) & 0x0f);
}

bool Sheep::mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);

    if (!player->isAllowedToInteract(shared_from_this()))
    {
        return false;
    }

    if (itemInstance->getItem() == Items::shears && !isSheared() && !isBaby())
    {
        if (!mLevel->mIsClientSide)
        {
            setSheared(true);

            int count = mRandom->nextInt(3) + 1;
            for (int i = 0; i < count; ++i)
            {
                boost::shared_ptr<ItemEntity> wool = spawnAtLocation(
                    not_null_ptr<ItemInstance>(new ItemInstance(Blocks::wool, 1, getColor()->getBlockData())), 1.0f);
                wool->mYd += mRandom->nextFloat() * 0.05f;
                wool->mXd += (mRandom->nextFloat() - mRandom->nextFloat()) * 0.1f;
                wool->mZd += (mRandom->nextFloat() - mRandom->nextFloat()) * 0.1f;
            }

            player->awardStat(GenericStats::shearedEntity(EntityType::SHEEP),
                              GenericStats::param_shearedEntity(EntityType::SHEEP));
        }

        itemInstance->hurtAndBreak(1, player);
        playSound(SoundEvents::SHEEP_SHEAR, 1.0f, 1.0f);
        return true;
    }

    return Animal::mobInteract(player, hand);
}

void Sheep::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::SHEEP);
}

void Sheep::addAdditonalSaveData(CompoundTag* tag)
{
    Animal::addAdditonalSaveData(tag);
    tag->putBoolean(L"Sheared", isSheared());
    tag->putByte(L"Color", static_cast<unsigned char>(getColor()->getBlockData()));
}

void Sheep::setColor(const DyeColor* color)
{
    unsigned char value = mEntityData->get(gSheepSheared);
    mEntityData->set(gSheepSheared, static_cast<unsigned char>((value & 0xf0) | (color->getBlockData() & 0x0f)), true);
}

void Sheep::readAdditionalSaveData(CompoundTag* tag)
{
    Animal::readAdditionalSaveData(tag);
    setSheared(tag->getBoolean(L"Sheared"));
    setColor(DyeColor::byBlockData(tag->getByte(L"Color")));
}

unsigned int Sheep::GetInteractTooltip(const EntityTooltipDataHolder& holder)
{
    switch (Item::getId(holder.m_pItemInstance->getItem()))
    {
    case ItemId::DYE:
    {
        const DyeColor* color = DyeColor::byItemData(holder.m_pItemInstance->getAuxValue());
        if (!isSheared() && getColor() != color)
        {
            return IDS_TOOLTIPS_DYE;
        }
        break;
    }
    case ItemId::SHEARS:
        if (!isBaby() && !isSheared())
        {
            return IDS_TOOLTIPS_SHEAR;
        }
        break;
    default:
        return Animal::GetInteractTooltip(holder);
    }

    return Animal::GetInteractTooltip(holder);
}

const SoundEvent* Sheep::getAmbientSound()
{
    return SoundEvents::SHEEP_AMBIENT;
}
const SoundEvent* Sheep::getHurtSound(const DamageSource*)
{
    return SoundEvents::SHEEP_HURT;
}
const SoundEvent* Sheep::getDeathSound()
{
    return SoundEvents::SHEEP_DEATH;
}

void Sheep::playStepSound(const BlockPos&, Block*)
{
    playSound(SoundEvents::SHEEP_STEP, 0.15f, 1.0f);
}

const DyeColor* Sheep::getRandomSheepColor(Random* random)
{
    int value = random->nextInt(100);
    if (value < 5)
        return DyeColor::BLACK;
    if (value < 10)
        return DyeColor::GRAY;
    if (value < 15)
        return DyeColor::SILVER;
    if (value < 18)
        return DyeColor::BROWN;
    return random->nextInt(500) == 0 ? DyeColor::PINK : DyeColor::WHITE;
}

const DyeColor* Sheep::getOffspringColor(boost::shared_ptr<Animal> first, boost::shared_ptr<Animal> second)
{
    int firstData = boost::dynamic_pointer_cast<Sheep>(first)->getColor()->getItemData();
    int secondData = boost::dynamic_pointer_cast<Sheep>(second)->getColor()->getItemData();

    mCraftingContainer->getItem(0)->setAuxValue(firstData);
    mCraftingContainer->getItem(1)->setAuxValue(secondData);

    not_null_ptr<ItemInstance> result = Recipes::getInstance()->getItemFor(mCraftingContainer, first->mLevel, NULL,
                                                                           Recipes::eRecipeCategory_Crafting);

    int itemData;
    if (result->getItem() == Items::dye)
    {
        itemData = result->getAuxValue();
    }
    else
    {
        itemData = mLevel->mRandom->nextBoolean() ? firstData : secondData;
    }

    return DyeColor::byItemData(itemData);
}

boost::shared_ptr<AgableMob> Sheep::getBreedOffspring(boost::shared_ptr<AgableMob> partner)
{
    boost::shared_ptr<Sheep> partnerSheep = boost::dynamic_pointer_cast<Sheep>(partner);
    boost::shared_ptr<Sheep> offspring(new Sheep(mLevel));
    offspring->setColor(getOffspringColor(boost::dynamic_pointer_cast<Animal>(shared_from_this()), partnerSheep));
    return offspring;
}

void Sheep::ate()
{
    setSheared(false);
    if (isBaby())
    {
        ageUp(60);
    }
}

MobGroupData* Sheep::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* group, const std::wstring&)
{
    group = Mob::finalizeMobSpawn(difficulty, group, L"");
    setColor(getRandomSheepColor(mLevel->mRandom));
    return group;
}

float Sheep::getEyeHeight()
{
    return 0.95f * mBbHeight;
}

unsigned long long Sheep::GetType()
{
    return EntityType::SHEEP;
}

bool Sheep::SheepContainer::stillValid(boost::shared_ptr<Player> player)
{
    return false;
}

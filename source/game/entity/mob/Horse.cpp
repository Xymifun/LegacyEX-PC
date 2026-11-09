#include "game/entity/mob/Horse.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/AttributeInstance.hpp"
#include "game/core/AttributeModifier.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/item/ItemDataWalker.hpp"
#include "game/entity/mob/HorseArmorItem.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/core/Mule.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/item/ItemId.hpp"
#include "game/world/block/BlockId.hpp"

const EntityDataAccessor<int>* Horse::DATA_VARIANT = NULL;
const EntityDataAccessor<not_null_ptr<ItemInstance> >* Horse::DATA_ARMOR = NULL;

const int Horse::VARIANT_TEXTURES[7] = {100, 92, 91, 90, 89, 94, 93};
const std::wstring Horse::VARIANT_HASHES[7] = {L"hwh", L"hcr", L"hch", L"hbr", L"hbl", L"hgr", L"hdb"};
const int Horse::MARKING_TEXTURES[5] = {-1, 96, 98, 97, 95};
const std::wstring Horse::MARKING_HASHES[5] = {L"", L"wo_", L"wmo", L"wdo", L"bdo"};
HorseArmorItem* Horse::ARMOR_NONE = new HorseArmorItem(0, 0);

Horse::HorseGroupData::HorseGroupData(int variant) : MobGroupData(), mVariant(variant)
{
}
Horse::HorseGroupData::~HorseGroupData()
{
}

Horse::Horse(Level* level) : AbstractHorse(level)
{
    fjDerivedCtorCalls();
    mLayeredTextureHashName = L"";
    mLayeredTextureLayers = arrayWithLength<int>(4, true);
    mLayeredTextureLayers.data[0] = -1;
    mLayeredTextureLayers.data[2] = -1;
    mLayeredTextureLayers.data[3] = -1;
    mLayeredTextureLayers.data[1] = -1;
    mLayeredTextureInfoBuilt = false;
}

Horse::~Horse()
{
    delete[] mLayeredTextureLayers.data;
}

Entity* Horse::create(Level* level)
{
    return new Horse(level);
}

HorseArmorItem* Horse::getArmorType()
{
    not_null_ptr<ItemInstance> item = mEntityData->get(DATA_ARMOR);
    HorseArmorItem* armor = ARMOR_NONE;
    if (HorseArmorItem::isHorseArmor(item->getItem()))
        armor = static_cast<HorseArmorItem*>(item->getItem());
    return armor;
}

void Horse::containerChanged(Container* container)
{
    HorseArmorItem* oldArmor = getArmorType();
    AbstractHorse::containerChanged(container);
    HorseArmorItem* newArmor = getArmorType();
    if (mTickCount > 20 && oldArmor != newArmor && newArmor->getTier() != 0)
        playSound(SoundEvents::HORSE_ARMOR, 0.5f, 1.0f);
}

void Horse::addDataWalkers(DataFixerUpper* fixer)
{
    AbstractHorse::addDataWalkers(fixer, EntityType::HORSE);
    fixer->addWalker(DataFixTypes::eDataFixTypes_Entity, new ItemDataWalker(EntityType::HORSE, L"ArmorItem"));
}
void Horse::staticCtor()
{
    DATA_VARIANT = SynchedEntityData::defineId<int>(EntityType::HORSE, EntityType::ABSTRACT_HORSE,
                                                    EntityDataSerializers::INT);
    DATA_ARMOR = SynchedEntityData::defineId<not_null_ptr<ItemInstance> >(EntityType::HORSE, EntityType::ABSTRACT_HORSE,
                                                                          EntityDataSerializers::ITEM_INSTANCE);
}
void Horse::defineSynchedData()
{
    AbstractHorse::defineSynchedData();
    mEntityData->define(DATA_VARIANT, 0);
    mEntityData->define(DATA_ARMOR, ItemInstance::EMPTY);
}
int Horse::getVariant()
{
    return mEntityData->get(DATA_VARIANT);
}

void Horse::addAdditonalSaveData(CompoundTag* tag)
{
    AbstractHorse::addAdditonalSaveData(tag);
    tag->putInt(L"Variant", getVariant());
    if (!mInventory->getItem(1)->isEmpty())
    {
        tag->put(L"ArmorItem", mInventory->getItem(1)->save(new CompoundTag()));
    }
}

void Horse::clearLayeredTextureInfo()
{
    mLayeredTextureHashName.erase(mLayeredTextureHashName.begin(), mLayeredTextureHashName.end());
}

void Horse::setVariant(int variant)
{
    mEntityData->set(DATA_VARIANT, variant, true);
    clearLayeredTextureInfo();
}

void Horse::readAdditionalSaveData(CompoundTag* tag)
{
    AbstractHorse::readAdditionalSaveData(tag);
    setVariant(tag->getInt(L"Variant"));
    if (tag->contains(L"ArmorItem", Tag::TAG_COMPOUND))
    {
        not_null_ptr<ItemInstance> armorItem(new ItemInstance(tag->getCompound(L"ArmorItem")));
        if (!armorItem->isEmpty() && HorseArmorItem::isHorseArmor(armorItem->getItem()))
        {
            mInventory->setItem(1, armorItem);
        }
    }
    updateEquipment();
}

void Horse::rebuildLayeredTextureInfo()
{
    mLayeredTextureHashName = L"horse/";
    for (unsigned int layer = 0; layer < mLayeredTextureLayers.length; ++layer)
        mLayeredTextureLayers[layer] = -1;

    int variant = getVariant();
    int colour = (variant & 0xFF) % 7;
    int markings = ((variant >> 8) & 0xFF) % 5;
    HorseArmorItem* armor = getArmorType();

    mLayeredTextureLayers[0] = VARIANT_TEXTURES[colour];
    mLayeredTextureLayers[1] = MARKING_TEXTURES[markings];
    int layerCount = mLayeredTextureLayers[1] == -1 ? 1 : 2;
    int armorTextureCount = armor->getTextureCount();
    for (int i = 0; i < armorTextureCount; ++i)
        mLayeredTextureLayers[layerCount + i] = armor->getTexture(i);

    mLayeredTextureHashName = L"horse/" + VARIANT_HASHES[colour] + MARKING_HASHES[markings] + armor->getHash();
    mLayeredTextureInfoBuilt = true;
}

std::wstring Horse::getLayeredTextureHashName()
{
    if (mLayeredTextureHashName.empty())
        rebuildLayeredTextureInfo();
    return mLayeredTextureHashName;
}

arrayWithLength<int> Horse::getLayeredTextureLayers()
{
    if (mLayeredTextureHashName.empty())
        rebuildLayeredTextureInfo();
    return mLayeredTextureLayers;
}

bool Horse::isWearingDyedArmor()
{
    return getArmorType() != ARMOR_NONE && getArmorType()->isDyeable();
}

int Horse::getDyedArmorColor()
{
    return getArmorType()->getDyedColor(mEntityData->get(DATA_ARMOR));
}

int Horse::getDyedArmorLayer()
{
    rebuildLayeredTextureInfo();
    if (mLayeredTextureLayers.data[3] == -1)
        return 1;
    return 2;
}

void Horse::setArmorType(const not_null_ptr<ItemInstance>& item)
{
    HorseArmorItem* armor = ARMOR_NONE;
    if (HorseArmorItem::isHorseArmor(item->getItem()))
        armor = static_cast<HorseArmorItem*>(item->getItem());

    mEntityData->set(DATA_ARMOR, item, true);
    clearLayeredTextureInfo();

    if (!mLevel->mIsClientSide)
    {
        getAttribute(SharedMonsterAttributes::ARMOR)->removeModifier(eMODIFIER_ID_ArmorBonus);
        int protection = armor->getProtection();
        if (protection != 0)
        {
            getAttribute(SharedMonsterAttributes::ARMOR)
                ->addModifier(
                    (new AttributeModifier(eMODIFIER_ID_ArmorBonus, (double)protection, 0))->setSerialize(false));
        }
    }
}

void Horse::updateEquipment()
{
    AbstractHorse::updateEquipment();
    not_null_ptr<ItemInstance> item = mInventory->getItem(1);
    setArmorType(item);
}

void Horse::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    AgableMob::onSyncedDataUpdated(accessor);
    if (accessor->getId() == DATA_ARMOR->getId())
        clearLayeredTextureInfo();
}

void Horse::playGallopSound(const SoundType* sound)
{
    AbstractHorse::playGallopSound(sound);
    if (mRandom->nextInt(10) == 0)
        playSound(SoundEvents::HORSE_BREATHE, sound->getVolume() * 0.6f, sound->getPitch());
}
void Horse::registerAttributes()
{
    AbstractHorse::registerAttributes();
    getAttributes()->registerAttribute(JUMP_STRENGTH);
    getAttribute(SharedMonsterAttributes::MAX_HEALTH)->setBaseValue(generateRandomMaxHealth());
    getAttribute(SharedMonsterAttributes::MOVEMENT_SPEED)->setBaseValue(generateRandomSpeed());
    getAttribute(JUMP_STRENGTH)->setBaseValue(generateRandomJumpStrength());
}

void Horse::tick()
{
    PIXBeginNamedEvent(0.0f, "Horse tick");
    AbstractHorse::tick();
    if (mLevel->mIsClientSide && mEntityData->isDirty())
    {
        mEntityData->clearDirty();
        clearLayeredTextureInfo();
    }
    PIXEndNamedEvent();
}

const SoundEvent* Horse::getAmbientSound()
{
    AbstractHorse::getAmbientSound();
    return SoundEvents::HORSE_AMBIENT;
}
const SoundEvent* Horse::getDeathSound()
{
    AbstractHorse::getDeathSound();
    return SoundEvents::HORSE_DEATH;
}
const SoundEvent* Horse::getHurtSound(const DamageSource* source)
{
    AbstractHorse::getHurtSound(source);
    return SoundEvents::HORSE_HURT;
}
const SoundEvent* Horse::getAngrySound()
{
    AbstractHorse::getAngrySound();
    return SoundEvents::HORSE_ANGRY;
}
ResourceLocation* Horse::getDefaultLootTable()
{
    return &BuiltInLootTables::HORSE;
}

bool Horse::mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> item = player->getItemInHand(hand);
    bool holdingItem = !item->isEmpty();
    if (holdingItem && item->getItem() == Items::spawnEgg)
        return AbstractHorse::mobInteract(player, hand);
    if (isLeashedToPlayer(player))
        return false;
    if (!isBaby())
    {
        if (isTamed() && player->isSneaking())
        {
            openInventory(player);
            return true;
        }
        if (isVehicle())
            return AbstractHorse::mobInteract(player, hand);
    }
    if (holdingItem)
    {
        if (handleEating(player, item))
        {
            if (!player->mAbilities.mInstabuild)
                item->shrink(1);
            return true;
        }
        if (item->interactEnemy(player, boost::static_pointer_cast<LivingEntity>(shared_from_this()), hand))
            return true;
        if (!isTamed())
        {
            makeMad();
            return true;
        }
        bool isHorseArmorItem = HorseArmorItem::isHorseArmor(item->getItem());
        bool isSaddleItem = !isBaby() && !isSaddled() && item->getItem() == Items::saddle;
        if (isHorseArmorItem || isSaddleItem)
        {
            openInventory(player);
            return true;
        }
    }
    if (isBaby())
        return AbstractHorse::mobInteract(player, hand);
    doPlayerRide(player);
    return true;
}

unsigned int Horse::GetInteractTooltip(const EntityTooltipDataHolder& tooltipData)
{
    if (!tooltipData.m_bAllowedToInteract)
        return IDS_NONE;

    unsigned int itemId = Item::getId(tooltipData.m_pItemInstance->getItem());
    bool food = itemId == BlockId::HAY_BLOCK || itemId == ItemId::APPLE || itemId == ItemId::WHEAT
                || itemId == ItemId::GOLDEN_APPLE || itemId == ItemId::SUGAR || itemId == ItemId::GOLDEN_CARROT;
    bool breedFood = itemId == ItemId::GOLDEN_CARROT || itemId == ItemId::GOLDEN_APPLE;
    bool horseArmor = itemId >= 417 && (itemId <= 419 || itemId == ItemId::LEATHER_HORSE_ARMOR);

    if (itemId == ItemId::SPAWN_EGG)
    {
        if (SpawnEggItem::fjGetType(tooltipData.m_pItemInstance) == EntityType::HORSE)
            return IDS_TOOLTIPS_PLACE;
        if (isLeashedToPlayer(tooltipData.m_pPlayer))
            return IDS_TOOLTIPS_UNLEASH;
        return IDS_NONE;
    }

    if (isLeashedToPlayer(tooltipData.m_pPlayer))
        return IDS_TOOLTIPS_UNLEASH;

    if (itemId == ItemId::LEAD)
    {
        if (!isLeashed())
            return IDS_TOOLTIPS_LEASH;
        if (!getPassengers()->empty() || !isTamed())
            return IDS_NONE;
        return IDS_TOOLTIPS_MOUNT;
    }

    if (itemId == ItemId::NAME_TAG && (!tooltipData.m_pPlayer->isSneaking() || !isTamed()))
    {
        if (tooltipData.m_pItemInstance->hasCustomHoverName()
            && (!hasCustomName() || tooltipData.m_pItemInstance->getHoverName().compare(getCustomName()) != 0))
            return IDS_TOOLTIPS_NAME;
        if (!isTamed() || !getPassengers()->empty())
            return IDS_NONE;
        return IDS_TOOLTIPS_MOUNT;
    }

    if (isBaby())
        return food ? IDS_TOOLTIPS_FEED : IDS_NONE;

    if (!isTamed())
    {
        if (!tooltipData.m_pItemInstance->isEmpty())
        {
            if (food && itemId != BlockId::HAY_BLOCK)
                return IDS_TOOLTIPS_FEED;
        }
        else if (getPassengers()->empty())
        {
            return IDS_TOOLTIPS_TAME;
        }
        return IDS_NONE;
    }

    if ((tooltipData.m_pPlayer->isSneaking() && (!isVehicle() || hasPassenger(tooltipData.m_pPlayer)) && isTamed())
        || (itemId == ItemId::SADDLE && !isSaddled()) || (wearsArmor() && horseArmor))
        return IDS_TOOLTIPS_OPEN;

    if (breedFood && !isInLove() && !isBaby() && getPassengers()->empty() && !isBreedCoolingDown())
        return IDS_TOOLTIPS_LOVEMODE;

    if (food && getHealth() < getMaxHealth())
        return IDS_TOOLTIPS_HEAL;

    if (getPassengers()->empty())
        return IDS_TOOLTIPS_MOUNT;
    return IDS_NONE;
}

bool Horse::HideUseTooltips()
{
    return true;
}

bool Horse::canMate(const boost::shared_ptr<Animal>& other)
{
    if (other == shared_from_this())
    {
        return false;
    }
    if (eTYPE_DERIVED_FROM(EntityType::DONKEY, other->GetType())
        || eTYPE_DERIVED_FROM(EntityType::HORSE, other->GetType()))
    {
        return canParent() && boost::static_pointer_cast<AbstractHorse>(other)->canParent();
    }
    return false;
}

boost::shared_ptr<AgableMob> Horse::getBreedOffspring(boost::shared_ptr<AgableMob> partner)
{
    boost::shared_ptr<AbstractHorse> child;
    bool partnerIsDonkey = eTYPE_DERIVED_FROM(EntityType::DONKEY, partner->GetType());
    if (partnerIsDonkey)
    {
        boost::shared_ptr<Mule> mule(new Mule(mLevel));
        child = mule;
    }
    else
    {
        boost::shared_ptr<Horse> partnerHorse = boost::static_pointer_cast<Horse>(partner);
        boost::shared_ptr<Horse> horse(new Horse(mLevel));
        child = horse;
        int colour;
        int roll = mRandom->nextInt(9);
        if (roll < 4)
        {
            colour = getVariant() & 0xff;
        }
        else if (roll < 8)
        {
            colour = partnerHorse->getVariant() & 0xff;
        }
        else
        {
            colour = mRandom->nextInt(7);
        }
        int markingRoll = mRandom->nextInt(5);
        if (markingRoll < 2)
        {
            colour |= getVariant() & 0xff00;
        }
        else if (markingRoll < 4)
        {
            colour |= partnerHorse->getVariant() & 0xff00;
        }
        else
        {
            colour |= (mRandom->nextInt(5) & 0xff) << 8;
        }
        boost::static_pointer_cast<Horse>(child)->setVariant(colour);
    }
    setOffspringAttributes(partner, child);
    return child;
}

bool Horse::wearsArmor()
{
    return true;
}

bool Horse::isArmor(const not_null_ptr<ItemInstance>& item)
{
    return HorseArmorItem::isHorseArmor(item->getItem());
}

MobGroupData* Horse::finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                      const std::wstring& name)
{
    MobGroupData* result = AbstractHorse::finalizeMobSpawn(difficulty, data, name);
    if (result != NULL && dynamic_cast<HorseGroupData*>(result) != NULL)
    {
        setVariant(static_cast<HorseGroupData*>(result)->mVariant | (mRandom->nextInt(5) << 8));
        return result;
    }
    int variant = mRandom->nextInt(7);
    result = new HorseGroupData(variant);
    setVariant(variant | (mRandom->nextInt(5) << 8));
    return result;
}

bool Horse::removeWhenFarAway()
{
    return AbstractHorse::removeWhenFarAway() && getArmorType() == ARMOR_NONE;
}

unsigned long long Horse::GetType()
{
    return EntityType::HORSE;
}

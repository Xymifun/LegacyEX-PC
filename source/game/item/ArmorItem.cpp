#include "game/item/ArmorItem.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <typeinfo>
#include <vector>

#include "game/client/audio/SoundEvent.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/AttributeModifier.hpp"
#include "game/core/BehaviorRegistry.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/InteractionResult.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/ai/EntitySelector.hpp"
#include "game/entity/ai/MobCanWearArmourEntitySelector.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/item/ArmorDispenseItemBehavior.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockSource.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/DispenserBlock.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

DispenseItemBehavior* ArmorItem::DISPENSE_ITEM_BEHAVIOR = new ArmorDispenseItemBehavior();

static const int HEALTH_PER_SLOT[4] = {13, 15, 16, 11};

static const eMODIFIER_ID ARMOR_MODIFIER_ID_PER_SLOT[4] = {eMODIFIER_ID_ArmorFeet, eMODIFIER_ID_ArmorLegs,
                                                           eMODIFIER_ID_ArmorChest, eMODIFIER_ID_ArmorHead};

static int LEATHER_SLOT_PROTECTIONS[4] = {1, 2, 3, 1};
static int CHAIN_SLOT_PROTECTIONS[4] = {1, 4, 5, 2};
static int IRON_SLOT_PROTECTIONS[4] = {2, 5, 6, 2};
static int GOLD_SLOT_PROTECTIONS[4] = {1, 3, 5, 2};
static int DIAMOND_SLOT_PROTECTIONS[4] = {3, 6, 8, 3};
static int TURTLE_SLOT_PROTECTIONS[4] = {2, 5, 6, 2};

static std::wstring OVERLAY_ICON_NAME_PER_SLOT[4] = {L"bootsCloth_overlay", L"leggingsCloth_overlay",
                                                     L"chestplateCloth_overlay", L"helmetCloth_overlay"};

const ArmorItem::ArmorMaterial* ArmorItem::ArmorMaterial::LEATHER;
const ArmorItem::ArmorMaterial* ArmorItem::ArmorMaterial::CHAIN;
const ArmorItem::ArmorMaterial* ArmorItem::ArmorMaterial::IRON;
const ArmorItem::ArmorMaterial* ArmorItem::ArmorMaterial::GOLD;
const ArmorItem::ArmorMaterial* ArmorItem::ArmorMaterial::DIAMOND;
const ArmorItem::ArmorMaterial* ArmorItem::ArmorMaterial::TURTLE;

ArmorItem::ArmorMaterial::ArmorMaterial(std::wstring name, int durabilityMultiplier, int* slotProtections,
                                        int enchantmentValue, const SoundEvent* equipSound, float toughness)
{
    mName = name;
    mDurabilityMultiplier = durabilityMultiplier;
    mSlotProtections = slotProtections;
    mEnchantmentValue = enchantmentValue;
    mEquipSound = equipSound;
    mToughness = toughness;
}

int ArmorItem::ArmorMaterial::getDefenseForSlot(const EquipmentSlot* slot) const
{
    return mSlotProtections[slot->getIndex()];
}

float ArmorItem::ArmorMaterial::getToughness() const
{
    return mToughness;
}

int ArmorItem::ArmorMaterial::getHealthForSlot(const EquipmentSlot* slot) const
{
    return HEALTH_PER_SLOT[slot->getIndex()] * mDurabilityMultiplier;
}

int ArmorItem::ArmorMaterial::getEnchantmentValue() const
{
    return mEnchantmentValue;
}

Item* ArmorItem::ArmorMaterial::getTierItem() const
{
    if (this == LEATHER)
    {
        return Items::leather;
    }

    if (this == CHAIN)
    {
        return Items::ironIngot;
    }

    if (this == GOLD)
    {
        return Items::goldIngot;
    }

    if (this == IRON)
    {
        return Items::ironIngot;
    }

    if (this == DIAMOND)
    {
        return Items::diamond;
    }

    return NULL;
}

const SoundEvent* ArmorItem::ArmorMaterial::getEquipSound() const
{
    return mEquipSound;
}

void ArmorItem::staticCtor()
{
    ArmorMaterial::LEATHER = new ArmorMaterial(L"leather", 5, LEATHER_SLOT_PROTECTIONS, 15,
                                               SoundEvents::ARMOR_EQUIP_LEATHER, 0.0f);
    ArmorMaterial::CHAIN = new ArmorMaterial(L"chainmail", 15, CHAIN_SLOT_PROTECTIONS, 12,
                                             SoundEvents::ARMOR_EQUIP_CHAIN, 0.0f);
    ArmorMaterial::IRON = new ArmorMaterial(L"iron", 15, IRON_SLOT_PROTECTIONS, 9, SoundEvents::ARMOR_EQUIP_IRON, 0.0f);
    ArmorMaterial::GOLD = new ArmorMaterial(L"gold", 7, GOLD_SLOT_PROTECTIONS, 25, SoundEvents::ARMOR_EQUIP_GOLD, 0.0f);
    ArmorMaterial::DIAMOND = new ArmorMaterial(L"diamond", 33, DIAMOND_SLOT_PROTECTIONS, 10,
                                               SoundEvents::ARMOR_EQUIP_DIAMOND, 2.0f);
    ArmorMaterial::TURTLE = new ArmorMaterial(L"turtle", 25, TURTLE_SLOT_PROTECTIONS, 9,
                                              SoundEvents::ARMOR_EQUIP_TURTLE, 0.0f);
}

not_null_ptr<ItemInstance> ArmorItem::dispenseArmor(BlockSource* blockSource, not_null_ptr<ItemInstance> itemInstance,
                                                    DefaultDispenseItemBehavior::eOUTCOME& outcome)
{
    BlockPos pos = blockSource->getPos().relative(blockSource->getBlockState()->getValue(DispenserBlock::FACING));

    AABB* area = AABB::newTemp(pos.x, pos.y, pos.z, pos.x + 1, pos.y + 1, pos.z + 1);

    Predicate<boost::shared_ptr<Entity> >* canWearArmour = new MobCanWearArmourEntitySelector(itemInstance);
    const Predicate<boost::shared_ptr<Entity> >* selectors[2] = {EntitySelector::NO_SPECTATORS, canWearArmour};
    Predicate<boost::shared_ptr<Entity> >* selector = new Predicates<boost::shared_ptr<Entity> >::ConnectivePredicate(
        Predicates<boost::shared_ptr<Entity> >::ConnectivePredicate::eOperator_And, selectors, 2);

    std::vector<boost::shared_ptr<Entity> >* entities = blockSource->getWorld()->getEntitiesOfClass(
        typeid(LivingEntity), area, selector, false);

    delete canWearArmour;
    delete selector;

    if (entities->empty())
    {
        delete entities;
        return ItemInstance::EMPTY;
    }

    boost::shared_ptr<LivingEntity> entity = boost::dynamic_pointer_cast<LivingEntity>(entities->at(0));
    const EquipmentSlot* slot = Mob::getEquipmentSlotForItem(itemInstance);

    entity->setItemSlot(slot, itemInstance->split(1));

    if (eTYPE_DERIVED_FROM(EntityType::MOB, entity->GetType()))
    {
        boost::static_pointer_cast<Mob>(entity)->setDropChance(slot, 2.0f);
    }

    outcome = DefaultDispenseItemBehavior::eOUTCOME_Success;

    delete entities;

    return itemInstance;
}

ArmorItem::ArmorItem(const ArmorMaterial* material, int renderIndex, const EquipmentSlot* slot)
{
    mSlot = slot;
    mDefense = material->getDefenseForSlot(slot);
    mToughness = material->getToughness();
    mRenderIndex = renderIndex;
    mMaterial = material;
    setMaxDamage(material->getHealthForSlot(slot));
    mMaxStackSize = 1;
    mSlot = slot;
    DispenserBlock::DISPENSER_REGISTRY.add(this, new ArmorDispenseItemBehavior());
}

int ArmorItem::getEnchantmentValue()
{
    return mMaterial->getEnchantmentValue();
}

const EquipmentSlot* ArmorItem::getSlot()
{
    return mSlot;
}

const ArmorItem::ArmorMaterial* ArmorItem::getMaterial()
{
    return mMaterial;
}

bool ArmorItem::hasCustomColor(not_null_ptr<ItemInstance> itemInstance)
{
    if (mMaterial == ArmorMaterial::LEATHER)
    {
        CompoundTag* tag = itemInstance->getTag();
        if (tag != NULL && tag->contains(L"display", Tag::TAG_COMPOUND))
        {
            return tag->getCompound(L"display")->contains(L"color", Tag::TAG_INT);
        }
    }

    return false;
}

int ArmorItem::getColor(not_null_ptr<ItemInstance> itemInstance, int layer)
{
    if (mMaterial != ArmorMaterial::LEATHER)
    {
        return 0xFFFFFF;
    }

    CompoundTag* tag = itemInstance->getTag();
    if (tag != NULL)
    {
        CompoundTag* display = tag->getCompound(L"display");
        if (display != NULL && display->contains(L"color", Tag::TAG_INT))
        {
            return display->getInt(L"color");
        }
    }

    return Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Armour_Default_Leather_Colour);
}

void ArmorItem::clearColor(not_null_ptr<ItemInstance> itemInstance)
{
    if (mMaterial == ArmorMaterial::LEATHER)
    {
        CompoundTag* tag = itemInstance->getTag();
        if (tag != NULL)
        {
            CompoundTag* display = tag->getCompound(L"display");
            if (display->contains(L"color"))
            {
                display->remove(L"color");
            }
        }
    }
}

void ArmorItem::setColor(not_null_ptr<ItemInstance> itemInstance, int color)
{
    if (mMaterial == ArmorMaterial::LEATHER)
    {
        CompoundTag* tag = itemInstance->getTag();
        if (tag == NULL)
        {
            tag = new CompoundTag();
            itemInstance->setTag(tag);
        }

        CompoundTag* display = tag->getCompound(L"display");
        if (!tag->contains(L"display"))
        {
            tag->put(L"display", display);
        }

        display->putInt(L"color", color);
    }
}

bool ArmorItem::isValidRepairItem(not_null_ptr<ItemInstance> itemInstance, not_null_ptr<ItemInstance> repairItem)
{
    if (mMaterial->getTierItem() == repairItem->getItem())
    {
        return true;
    }

    return Item::isValidRepairItem(itemInstance, repairItem);
}

InteractionResultHolder ArmorItem::use(Level* level, boost::shared_ptr<Player> player,
                                       InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInHand = player->getItemInHand(hand);
    const EquipmentSlot* slot = Mob::getEquipmentSlotForItem(itemInHand);
    not_null_ptr<ItemInstance> equippedItem = player->getItemSlot(slot);

    if ((!equippedItem->isEmpty() && slot->getEIndex() == EquipmentSlot::eIndex_Chest
         && equippedItem->getItem() == Items::elytra && player->isFallFlying())
        || EnchantmentHelper::hasBindingCurse(equippedItem))
    {
        return InteractionResultHolder(InteractionResult::eInteractionResult_Fail, itemInHand);
    }

    player->setItemSlot(slot, itemInHand->copy());
    player->OnEquipArmor(itemInHand);
    itemInHand->setCount(0);

    if (equippedItem->isEmpty())
    {
        return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInHand);
    }

    return InteractionResultHolder(InteractionResult::eInteractionResult_Success, equippedItem);
}

bool ArmorItem::TestUse(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand)
{
    const EquipmentSlot* slot = Mob::getEquipmentSlotForItem(player->getItemInHand(hand));
    not_null_ptr<ItemInstance> equippedItem = player->getItemSlot(slot);

    if ((!equippedItem->isEmpty() && slot->getEIndex() == EquipmentSlot::eIndex_Chest
         && equippedItem->getItem() == Items::elytra && player->isFallFlying())
        || EnchantmentHelper::hasBindingCurse(equippedItem))
    {
        return false;
    }

    return true;
}

unsigned int ArmorItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    if (tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Fail)
    {
        return IDS_NONE;
    }

    const EquipmentSlot* slot = getSlot();
    not_null_ptr<ItemInstance> equippedItem = tooltipData.m_pPlayer->getItemSlot(slot);

    if ((!equippedItem->isEmpty() && slot->getEIndex() == EquipmentSlot::eIndex_Chest
         && equippedItem->getItem() == Items::elytra && tooltipData.m_pPlayer->isFallFlying())
        || EnchantmentHelper::hasBindingCurse(equippedItem))
    {
        return IDS_NONE;
    }

    return IDS_TOOLTIPS_EQUIP;
}

boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*>* ArmorItem::getDefaultAttributeModifiers(
    const EquipmentSlot* slot)
{
    boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*>* modifiers = Item::getDefaultAttributeModifiers(slot);

    if (slot == mSlot)
    {
        (*modifiers)[SharedMonsterAttributes::ARMOR->getId()] = new AttributeModifier(
            ARMOR_MODIFIER_ID_PER_SLOT[slot->getIndex()], (double)mDefense, 0);
        (*modifiers)[SharedMonsterAttributes::ARMOR_TOUGHNESS->getId()] = new AttributeModifier(
            ARMOR_MODIFIER_ID_PER_SLOT[slot->getIndex()], (double)mToughness, 0);
    }

    return modifiers;
}

int ArmorItem::GetArmorType()
{
    const EquipmentSlot* slot = mSlot;

    if (slot == EquipmentSlot::HEAD)
    {
        return 1;
    }

    if (slot == EquipmentSlot::CHEST)
    {
        return 2;
    }

    if (slot == EquipmentSlot::LEGS)
    {
        return 3;
    }

    if (slot == EquipmentSlot::FEET)
    {
        return 4;
    }

    return 1;
}

bool ArmorItem::hasMultipleSpriteLayers()
{
    return mMaterial == ArmorMaterial::LEATHER;
}

TextureAtlasSprite* ArmorItem::getLayerIcon(int auxValue, int layer, not_null_ptr<ItemInstance> itemInstance)
{
    if (layer == 1)
    {
        return mOverlayIcon;
    }

    return Item::getLayerIcon(auxValue, layer, itemInstance);
}

void ArmorItem::registerIcons(IconRegister* iconRegister)
{
    Item::registerIcons(iconRegister);

    if (mMaterial == ArmorMaterial::LEATHER)
    {
        mOverlayIcon = iconRegister->registerIcon(OVERLAY_ICON_NAME_PER_SLOT[mSlot->getIndex()]);
    }
}

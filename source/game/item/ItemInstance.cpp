#include "game/item/ItemInstance.hpp"

const wchar_t* ItemInstance::TAG_ENCH = L"ench";
const wchar_t* ItemInstance::TAG_ENCH_ID = L"id";
const wchar_t* ItemInstance::TAG_ENCH_LEVEL = L"lvl";
const wchar_t* ItemInstance::TAG_DISPLAY = L"display";
const wchar_t* ItemInstance::TAG_NAME = L"Name";
const wchar_t* ItemInstance::TAG_LOC_NAME = L"LocName";
const wchar_t* ItemInstance::TAG_UNBREAKABLE = L"Unbreakable";
const wchar_t* ItemInstance::TAG_REPAIR_COST = L"RepairCost";
const wchar_t* ItemInstance::TAG_OWNER_ID = L"ownerID";
const wchar_t* ItemInstance::TAG_CREATED_ON_HOST = L"createdOnHost";
const wchar_t* ItemInstance::TAG_CREATED_BY_RESTRICTED = L"createdByRestricted";
const wchar_t* ItemInstance::TAG_FISH_BUCKET = L"FishBucket";
const wchar_t* ItemInstance::TAG_CAN_DESTROY = L"CanDestroy";
const wchar_t* ItemInstance::TAG_CAN_PLACE_ON = L"CanPlaceOn";
const wchar_t* ItemInstance::TAG_HIDE_FLAGS = L"HideFlags";
#include "game/core/StringIds.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/Tag.hpp"
#include "game/data/IntTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/data/DataFixTypes.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/entity/mob/ItemInstanceEntityTagWalker.hpp"
#include "game/world/block/ItemInstanceBlockEntityTagWalker.hpp"
#include "game/item/Enchantment.hpp"
#include "game/core/Rarity.hpp"
#include "game/core/L10N.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/DigDurabilityEnchantment.hpp"
#include "game/item/Enchantments.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include <cwchar>
#include <sstream>
#include <iomanip>
#include "game/core/AttributeModifier.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/StringConversion.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/HoeItem.hpp"
#include "game/item/NonNullList.hpp"
#include "game/item/PickaxeItem.hpp"
#include "game/item/Recipes.hpp"
#include "game/item/SwordItem.hpp"
#include "game/ui/CraftingMenu.hpp"
#include "game/ui/InventoryMenu.hpp"
#include "game/world/block/Blocks.hpp"
const not_null_ptr<ItemInstance> ItemInstance::EMPTY(new ItemInstance(static_cast<Item*>(NULL)));

ItemInstance::ItemInstance(Block* block) : mFrame(), mCensorshipInfo()
{
    _init(Item::byBlock(block), 1, 0);
}

ItemInstance::ItemInstance(Block* block, int count) : mFrame(), mCensorshipInfo()
{
    Item* item = Item::byBlock(block);
    _init(item, count, block->convertBlockStateToLegacyData(block->defaultBlockState()));
}

ItemInstance::ItemInstance(Block* block, int count, int auxValue) : mFrame(), mCensorshipInfo()
{
    _init(Item::byBlock(block), count, auxValue);
}

ItemInstance::ItemInstance(Item* item) : mFrame(), mCensorshipInfo()
{
    _init(item, 1, 0);
}

ItemInstance::ItemInstance(Item* item, int count) : mFrame(), mCensorshipInfo()
{
    _init(item, count, 0);
}

ItemInstance::ItemInstance(Item* item, int count, int auxValue) : mFrame(), mCensorshipInfo()
{
    _init(item, count, auxValue);
}

ItemInstance::ItemInstance(CompoundTag* tag) : mFrame(), mCensorshipInfo()
{
    _init(NULL, 0, 0);
    mItem = Item::byString(tag->getString(L"id"));
    mPopTime = 0;
    mCount = tag->getByte(L"Count");
    mAuxValue = tag->getShort(L"Damage");
    if (mAuxValue < 0)
        mAuxValue = 0;
    if (tag->contains(L"tag", Tag::TAG_COMPOUND))
    {
        delete mTag;
        mTag = static_cast<CompoundTag*>(tag->getCompound(L"tag")->copy());
        if (mItem != NULL)
            mItem->verifyTagAfterLoad(mTag);
    }
    updateEmptyCacheFlag();
}

ItemInstance::ItemInstance(const ItemInstance& other)
    : mCount(other.mCount),
      mPopTime(other.mPopTime),
      mItem(other.mItem),
      mTag(other.mTag),
      mEmptyCache(other.mEmptyCache),
      mAuxValue(other.mAuxValue),
      mUnknown(other.mUnknown),
      mFrame(other.mFrame),
      mCensorshipInfo(other.mCensorshipInfo),
      mBreakBlock(other.mBreakBlock),
      mCanBreak(other.mCanBreak),
      mPlaceBlock(other.mPlaceBlock),
      mCanPlace(other.mCanPlace)
{
}

ItemInstance::~ItemInstance()
{
    delete mTag;
}

ItemInstance& ItemInstance::operator=(const ItemInstance& other)
{
    mCount = other.mCount;
    mPopTime = other.mPopTime;
    mItem = other.mItem;
    mTag = other.mTag;
    mEmptyCache = other.mEmptyCache;
    mAuxValue = other.mAuxValue;
    mUnknown = other.mUnknown;
    mFrame = other.mFrame;
    mCensorshipInfo = other.mCensorshipInfo;
    mBreakBlock = other.mBreakBlock;
    mCanBreak = other.mCanBreak;
    mPlaceBlock = other.mPlaceBlock;
    mCanPlace = other.mCanPlace;
    return *this;
}

bool ItemInstance::isEmpty() const
{
    if (this == EMPTY.get() || mItem == NULL || mItem == Items::air || mCount <= 0)
    {
        return true;
    }
    return false;
}

int ItemInstance::getPopTime() const
{
    return mPopTime;
}

int ItemInstance::getCount()
{
    return mEmptyCache != 0 ? 0 : mCount;
}

void ItemInstance::updateEmptyCacheFlag()
{
    mEmptyCache = isEmpty() ? 1 : 0;
}

void ItemInstance::setCount(int count)
{
    mCount = count;
    updateEmptyCacheFlag();
}

Item* ItemInstance::getItem() const
{
    return mEmptyCache ? Items::air : mItem;
}

void ItemInstance::_init(Item* item, int count, int auxValue)
{
    mItem = item;
    mUnknown = 0;
    mTag = 0;
    mCount = count;
    mPopTime = 0;
    mAuxValue = auxValue;
    mCensorshipInfo = boost::shared_ptr<CensoredString>(
        new CensoredString(IDS_STRINGVERIFY_AWAITING_APPROVAL, IDS_STRINGVERIFY_CENSORED));
    mCanBreak = 0;
    mBreakBlock = 0;
    updateEmptyCacheFlag();
}

void ItemInstance::addDataWalkers(DataFixerUpper* fixer)
{
    fixer->addWalker(DataFixTypes::eDataFixTypes_ItemInstance, new ItemInstanceBlockEntityTagWalker());
    fixer->addWalker(DataFixTypes::eDataFixTypes_ItemInstance, new ItemInstanceEntityTagWalker());
}

not_null_ptr<ItemInstance> ItemInstance::copy() const
{
    not_null_ptr<ItemInstance> result(new ItemInstance(mItem, mCount, mAuxValue));
    if (mTag != 0)
    {
        result->mTag = mTag->copy();
    }
    return result;
}

void ItemInstance::grow(int amount)
{
    setCount(mCount + amount);
}

void ItemInstance::shrink(int amount)
{
    grow(-amount);
}

not_null_ptr<ItemInstance> ItemInstance::split(int amount)
{
    if (mCount < amount)
    {
        amount = mCount;
    }
    not_null_ptr<ItemInstance> result = copy();
    result->setCount(amount);
    shrink(amount);
    if (mCount < 1)
    {
        mCount = 0;
    }
    return result;
}

InteractionResult::EInteractionResult ItemInstance::useOn(boost::shared_ptr<Player> player, Level* level,
                                                          const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                          const Direction* direction, float x, float y, float z,
                                                          bool flag)
{
    InteractionResult::EInteractionResult result = getItem()->useOn(player, level, pos, hand, direction, x, y, z, flag);
    if (result == InteractionResult::eInteractionResult_Success)
    {
        Player* awardee = player.operator->();
        Stat* stat = gStats->get_itemsUsed(Item::VALIDgetId(mItem));
        awardee->awardStat(stat, GenericStats::param_itemsUsed(player, not_null_ptr<ItemInstance>(shared_from_this())));
    }
    return result;
}

float ItemInstance::getDestroySpeed(const BlockState* state)
{
    return getItem()->getDestroySpeed(not_null_ptr<ItemInstance>(shared_from_this()), state);
}

bool ItemInstance::TestUse(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand)
{
    return getItem()->TestUse(level, player, hand);
}

InteractionResultHolder ItemInstance::use(Level* level, boost::shared_ptr<Player> player,
                                          InteractionHand::EInteractionHand hand)
{
    return getItem()->use(level, player, hand);
}

not_null_ptr<ItemInstance> ItemInstance::finishUsingItem(Level* level, boost::shared_ptr<Player> player)
{
    return getItem()->finishUsingItem(not_null_ptr<ItemInstance>(shared_from_this()), level, player);
}

CompoundTag* ItemInstance::save(CompoundTag* tag)
{
    ResourceLocation location = Item::itemRegistry.getKey(mItem);
    tag->putString(L"id", location.toString());
    tag->putByte(L"Count", (unsigned char)mCount);
    tag->putShort(L"Damage", (short)mAuxValue);
    if (mTag != 0)
        tag->put(L"tag", mTag->copy());
    return tag;
}

int ItemInstance::getMaxStackSize()
{
    return getItem()->getMaxStackSize();
}

bool ItemInstance::hasTag()
{
    return mEmptyCache == 0 && mTag != 0;
}

CompoundTag* ItemInstance::getTag()
{
    return mTag;
}

bool ItemInstance::isDamageableItem()
{
    if (mEmptyCache == 0 && mItem->getMaxDamage() > 0)
    {
        bool unbreakable = false;
        if (hasTag())
        {
            unbreakable = getTag()->getBoolean(TAG_UNBREAKABLE);
        }
        return !unbreakable;
    }
    return false;
}

bool ItemInstance::isDamaged()
{
    return isDamageableItem() && mAuxValue > 0;
}

bool ItemInstance::isStackable()
{
    return getMaxStackSize() > 1 && (!isDamageableItem() || !isDamaged());
}

bool ItemInstance::isStackedByData()
{
    return getItem()->isStackedByData();
}

int ItemInstance::getDamageValue()
{
    return mAuxValue;
}

int ItemInstance::getAuxValue() const
{
    return mAuxValue;
}

void ItemInstance::setAuxValue(int auxValue)
{
    if (auxValue < 0)
        auxValue = 0;
    mAuxValue = auxValue;
}

int ItemInstance::getMaxDamage()
{
    return getItem()->getMaxDamage();
}

bool ItemInstance::hurt(int amount, Random* random)
{
    if (!isDamageableItem())
    {
        return false;
    }
    if (amount > 0)
    {
        not_null_ptr<ItemInstance> self;
        static_cast<boost::shared_ptr<ItemInstance>&>(self) = shared_from_this();
        int level = EnchantmentHelper::getItemEnchantmentLevel(Enchantments::UNBREAKING, self);
        int ignored = 0;
        int i = 0;
        if (level > 0 && amount > 0)
        {
            do
            {
                not_null_ptr<ItemInstance> item;
                static_cast<boost::shared_ptr<ItemInstance>&>(item) = shared_from_this();
                if (DigDurabilityEnchantment::shouldIgnoreDurabilityDrop(item, level, random))
                {
                    ++ignored;
                }
                ++i;
            } while (i < amount);
        }
        amount -= ignored;
        if (amount < 1)
        {
            return false;
        }
    }
    mAuxValue += amount;
    return getMaxDamage() < mAuxValue;
}

void ItemInstance::hurtAndBreak(int amount, boost::shared_ptr<LivingEntity> entity)
{
    boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(entity);
    if (player && player->mAbilities.mInstabuild)
        return;
    if (!isDamageableItem())
        return;
    if (Minecraft::InMiniGame(eMiniGameId_Tumble, true) || Minecraft::InMiniGame(eMiniGameId_Glide, true))
        return;
    if (hurt(amount, entity->getRandom()))
    {
        entity->breakItem(not_null_ptr<ItemInstance>(shared_from_this()));
        shrink(1);
        mAuxValue = 0;
    }
}

void ItemInstance::hurtEnemy(boost::shared_ptr<LivingEntity> entity, boost::shared_ptr<Player> player)
{
    getItem()->hurtEnemy(not_null_ptr<ItemInstance>(shared_from_this()), entity, player);
}

void ItemInstance::mineBlock(Level* level, const BlockState* state, const BlockPos& pos,
                             boost::shared_ptr<Player> player)
{
    getItem()->mineBlock(not_null_ptr<ItemInstance>(shared_from_this()), level, state, pos, player);
}

bool ItemInstance::canDestroySpecial(const BlockState* state)
{
    return getItem()->canDestroySpecial(state);
}

bool ItemInstance::interactEnemy(boost::shared_ptr<Player> player, boost::shared_ptr<LivingEntity> entity,
                                 InteractionHand::EInteractionHand hand)
{
    return getItem()->interactEnemy(not_null_ptr<ItemInstance>(shared_from_this()), player, entity, hand);
}

ItemInstance* ItemInstance::copy_not_shared() const
{
    ItemInstance* result = new ItemInstance(mItem, mCount, mAuxValue);
    if (mTag != 0)
        result->mTag = static_cast<CompoundTag*>(mTag->copy());
    return result;
}

bool ItemInstance::tagMatches(const not_null_ptr<ItemInstance>& first, const not_null_ptr<ItemInstance>& second)
{
    if (first->mEmptyCache && second->mEmptyCache)
        return true;
    if (first->mEmptyCache || second->mEmptyCache)
        return false;
    if (first->mTag == 0 && second->mTag != 0)
        return false;
    if (first->mTag != 0 && !first->mTag->equals(second->mTag))
        return false;
    return true;
}

bool ItemInstance::matches(const not_null_ptr<ItemInstance>& other, bool compareAuxValue)
{
    if (mCount != other->mCount)
        return false;
    if (getItem() != other->getItem())
        return false;
    if (compareAuxValue && mAuxValue != other->mAuxValue)
        return false;
    if (mTag == 0 && other->mTag != 0)
        return false;
    if (mTag != 0 && !mTag->equals(other->mTag))
        return false;
    return true;
}

bool ItemInstance::matches(const not_null_ptr<ItemInstance>& first, const not_null_ptr<ItemInstance>& second,
                           bool compareAuxValue)
{
    if (first->isEmpty() && second->isEmpty())
        return true;
    if (first->isEmpty() || second->isEmpty())
        return false;
    return first->matches(second, compareAuxValue);
}

bool ItemInstance::sameItem(const not_null_ptr<ItemInstance>& other)
{
    return !other->isEmpty() && mItem == other->mItem && mAuxValue == other->mAuxValue;
}

bool ItemInstance::isSame(const not_null_ptr<ItemInstance>& first, const not_null_ptr<ItemInstance>& second)
{
    if (first.get() == second.get())
    {
        return true;
    }
    if (!first->isEmpty() && !second->isEmpty())
    {
        return first->sameItem(second);
    }
    return false;
}

bool ItemInstance::sameItemIgnoreDurability(const not_null_ptr<ItemInstance>& other)
{
    if (isDamageableItem())
        return !other->isEmpty() && mItem == other->mItem;
    return sameItem(other);
}

bool ItemInstance::isSameIgnoreDurability(const not_null_ptr<ItemInstance>& first,
                                          const not_null_ptr<ItemInstance>& second)
{
    if (first.get() == second.get())
    {
        return true;
    }
    if (!first->isEmpty() && !second->isEmpty())
    {
        return first->sameItemIgnoreDurability(second);
    }
    return false;
}

bool ItemInstance::sameItemWithTags(const not_null_ptr<ItemInstance>& other, std::vector<std::wstring>* tags)
{
    if (mItem != other->mItem)
        return false;
    if (mAuxValue != other->mAuxValue)
        return false;
    if (mTag == 0 && other->mTag != 0)
        return false;
    if (mTag != 0 && !(tags != 0 ? mTag->equalsWithException(other->mTag, tags) : mTag->equals(other->mTag)))
        return false;
    return true;
}

bool ItemInstance::sameItem_not_shared(ItemInstance* other)
{
    return !isEmpty() && mItem == other->mItem && mAuxValue == other->mAuxValue;
}

int ItemInstance::getUseDescriptionId()
{
    return getItem()->getUseDescriptionId(not_null_ptr<ItemInstance>(shared_from_this()));
}

int ItemInstance::getDescriptionId(int auxValue)
{
    return getItem()->getDescriptionId(not_null_ptr<ItemInstance>(shared_from_this()));
}

std::wstring ItemInstance::toString()
{
    std::wostringstream stream;
    stream.setf(std::ios::dec, std::ios::basefield);
    stream << mCount << L"x" << getItem()->getDescription(not_null_ptr<ItemInstance>(shared_from_this())) << L"@"
           << mAuxValue;
    return stream.str();
}

void ItemInstance::inventoryTick(Level* level, boost::shared_ptr<Entity> entity, int slot, bool selected)
{
    if (mPopTime > 0)
    {
        --mPopTime;
    }
    if (mItem != 0)
    {
        mItem->inventoryTick(not_null_ptr<ItemInstance>(shared_from_this()), level, entity, slot, selected);
    }
}

void ItemInstance::onCraftedBy(Level* level, boost::shared_ptr<Player> player, int amount)
{
    bool countCraft = true;
    player->onCrafted(shared_from_this());

    InventoryMenu* inventoryMenu = dynamic_cast<InventoryMenu*>(player->mContainerMenu);
    CraftingMenu* craftingMenu = dynamic_cast<CraftingMenu*>(player->mContainerMenu);
    boost::shared_ptr<CraftingContainer> craftSlots;
    if (inventoryMenu != NULL)
        craftSlots = inventoryMenu->mCraftSlots;
    if (craftingMenu != NULL)
        craftSlots = craftingMenu->m_craftSlots;

    if (craftSlots != NULL)
    {
        Recipes* recipes = Recipes::getInstance();
        if (recipes->hasAchievementAttachedToRecipe(craftSlots, player->mLevel, NULL, Recipes::eRecipeCategory_Crafting)
            == eAward_ChangeOfSheets)
        {
            player->awardStat(GenericStats::changeOfSheets(), GenericStats::param_changeOfSheets());
        }

        NonNullList<not_null_ptr<ItemInstance> > remainingItems = recipes->getRemainingItemsFor(
            craftSlots, player->mLevel, Recipes::eRecipeCategory_Crafting);
        bool pickaxeRemains = false;
        bool hoeRemains = false;
        bool swordRemains = false;
        for (unsigned int index = 0; index < remainingItems.size(); ++index)
        {
            not_null_ptr<ItemInstance> remaining = craftSlots->getItem(index);
            if (dynamic_cast<PickaxeItem*>(remaining->getItem()) != NULL)
                pickaxeRemains = true;
            if (dynamic_cast<HoeItem*>(remaining->getItem()) != NULL)
                hoeRemains = true;
            if (dynamic_cast<SwordItem*>(remaining->getItem()) != NULL)
                swordRemains = true;
        }

        Item* item = getItem();
        const bool isPickaxe = dynamic_cast<PickaxeItem*>(item) != NULL;
        const bool isHoe = dynamic_cast<HoeItem*>(item) != NULL;
        const bool isSword = dynamic_cast<SwordItem*>(item) != NULL;
        if ((isPickaxe && pickaxeRemains) || (isHoe && hoeRemains) || (isSword && swordRemains))
            countCraft = false;

        if (item == Item::byBlock(Blocks::crafting_table))
        {
            player->awardStat(GenericStats::buildWorkbench(), GenericStats::param_buildWorkbench());
        }
        else if (isPickaxe && !pickaxeRemains)
        {
            if (item != Items::woodenPickaxe)
                player->awardStat(GenericStats::buildBetterPickaxe(), GenericStats::param_buildBetterPickaxe());
            else
                player->awardStat(GenericStats::buildPickaxe(), GenericStats::param_buildPickaxe());
        }
        else if (item == Item::byBlock(Blocks::furnace))
        {
            player->awardStat(GenericStats::buildFurnace(), GenericStats::param_buildFurnace());
        }
        else if (isHoe && !hoeRemains)
        {
            player->awardStat(GenericStats::buildHoe(), GenericStats::param_buildHoe());
        }
        else if (item == Items::bread)
        {
            player->awardStat(GenericStats::makeBread(), GenericStats::param_makeBread());
        }
        else if (item == Items::cake)
        {
            player->awardStat(GenericStats::bakeCake(), GenericStats::param_bakeCake());
        }
        else if (isSword && !swordRemains)
        {
            player->awardStat(GenericStats::buildSword(), GenericStats::param_buildSword());
        }
        else if (item == Item::byBlock(Blocks::enchanting_table))
        {
            player->awardStat(GenericStats::enchantments(), GenericStats::param_enchantments());
        }
        else if (item == Item::byBlock(Blocks::bookshelf))
        {
            player->awardStat(GenericStats::bookcase(), GenericStats::param_bookcase());
        }
        else if (Block::byItem(item) == Blocks::dispenser)
        {
            player->awardStat(GenericStats::dispenseWithThis(), GenericStats::param_dispenseWithThis());
        }
    }

    if (countCraft)
    {
        player->awardStat(GenericStats::itemsCrafted(mItem),
                          GenericStats::param_itemsCrafted(mItem, mAuxValue, amount));
    }
    getItem()->onCraftedBy(shared_from_this(), level, player);
}

int ItemInstance::getUseDuration()
{
    return getItem()->getUseDuration(not_null_ptr<ItemInstance>(shared_from_this()));
}

int ItemInstance::getUseAnimation() const
{
    return getItem()->getUseAnimation(not_null_ptr<const ItemInstance>(shared_from_this()));
}

void ItemInstance::releaseUsing(Level* level, boost::shared_ptr<LivingEntity> entity, int duration)
{
    getItem()->releaseUsing(not_null_ptr<ItemInstance>(shared_from_this()), level, entity, duration);
}

void ItemInstance::setTag(CompoundTag* tag)
{
    if (mTag != 0)
        delete mTag;
    mTag = tag;
}

void ItemInstance::addTagElement(std::wstring name, Tag* tag)
{
    if (mTag == 0)
        setTag(new CompoundTag());
    mTag->put(name.c_str(), tag);
}

Tag* ItemInstance::getOrCreateTagElement(const std::wstring& name)
{
    if (mTag != 0 && mTag->contains(name, Tag::TAG_COMPOUND))
        return mTag->getCompound(name);
    CompoundTag* tag = new CompoundTag();
    addTagElement(name, tag);
    return tag;
}

Tag* ItemInstance::getTagElement(const std::wstring& name)
{
    return mTag != 0 && mTag->contains(name, Tag::TAG_COMPOUND) ? mTag->getCompound(name) : 0;
}

ListTag* ItemInstance::getEnchantmentTags()
{
    if (mTag != 0 && mTag->contains(TAG_ENCH))
        return mTag->getList(TAG_ENCH, Tag::TAG_COMPOUND);
    return 0;
}

bool ItemInstance::wasCreatedByRestrictedPlayer()
{
    if (mTag == 0 || !mTag->contains(TAG_CREATED_BY_RESTRICTED))
        return false;
    return mTag->getBoolean(TAG_CREATED_BY_RESTRICTED);
}

std::wstring ItemInstance::getHoverNameUnfiltered()
{
    CompoundTag* display = static_cast<CompoundTag*>(getTagElement(TAG_DISPLAY));
    if (display != 0)
    {
        if (display->contains(TAG_NAME))
        {
            return display->getString(TAG_NAME);
        }
        if (display->contains(TAG_LOC_NAME, Tag::TAG_INT))
        {
            return L10N::GetString(display->getInt(TAG_LOC_NAME));
        }
    }
    not_null_ptr<ItemInstance> self;
    static_cast<boost::shared_ptr<ItemInstance>&>(self) = shared_from_this();
    return getItem()->getName(self);
}

std::wstring ItemInstance::getHoverName()
{
    if (isEmpty())
    {
        return std::wstring(L"[NULL]");
    }
    CompoundTag* display = static_cast<CompoundTag*>(getTagElement(TAG_DISPLAY));
    bool hasName = display != 0 && display->contains(TAG_NAME, Tag::TAG_STRING);
    if (!hasName)
    {
        return getHoverNameUnfiltered();
    }
    std::wstring name = display->getString(TAG_NAME);
    std::wstring ownerUUID = mTag->getString(TAG_OWNER_ID);
    bool restricted = wasCreatedByRestrictedPlayer();
#ifdef MINECRAFT_PC
    restricted = false;
#endif
    mCensorshipInfo->set(name, ownerUUID, restricted, mTag->getByte(TAG_CREATED_ON_HOST) != 0);
    if (mCensorshipInfo->checkParentalFilter() == 0)
    {
        not_null_ptr<ItemInstance> self;
        static_cast<boost::shared_ptr<ItemInstance>&>(self) = shared_from_this();
        return getItem()->getName(self);
    }
    return mCensorshipInfo->get();
}

not_null_ptr<ItemInstance> ItemInstance::setTranslatableName(int name)
{
    static_cast<CompoundTag*>(getOrCreateTagElement(TAG_DISPLAY))->putInt(TAG_LOC_NAME, name);
    return not_null_ptr<ItemInstance>(shared_from_this());
}

not_null_ptr<ItemInstance> ItemInstance::setHoverName(const std::wstring& name)
{
    CompoundTag* display = static_cast<CompoundTag*>(getOrCreateTagElement(TAG_DISPLAY));
    display->putString(TAG_NAME, name);
    not_null_ptr<ItemInstance> self;
    static_cast<boost::shared_ptr<ItemInstance>&>(self) = shared_from_this();
    return self;
}

void ItemInstance::setCensorshipInfo(const std::wstring& name, bool censored)
{
    if (mTag == 0)
        mTag = new CompoundTag();
    mTag->putString(TAG_OWNER_ID, name);
    mTag->putBoolean(TAG_CREATED_BY_RESTRICTED, censored);
    bool createdOnHost = false;
    if (CGameNetworkManager::sInstance.IsHost())
    {
        Minecraft* minecraft = Minecraft::GetInstance();
        for (int i = 0; minecraft != 0 && i < 4; ++i)
        {
            boost::shared_ptr<MultiplayerLocalPlayer> local = minecraft->mLocalPlayers[i];
            if (local != 0 && local->getUUID() == name)
            {
                createdOnHost = true;
                break;
            }
        }
    }
    mTag->putByte(TAG_CREATED_ON_HOST, createdOnHost ? 1 : 0);
}

std::wstring ItemInstance::getOwnerUUID()
{
    if (mTag != 0 && mTag->contains(TAG_OWNER_ID))
    {
        return mTag->getString(TAG_OWNER_ID);
    }
    return std::wstring();
}

bool ItemInstance::wasCreatedOnHost()
{
    if (mTag == 0 || !mTag->contains(TAG_CREATED_ON_HOST))
        return true;
    return mTag->getByte(TAG_CREATED_ON_HOST) == 1;
}

void ItemInstance::resetHoverName()
{
    CompoundTag* display = static_cast<CompoundTag*>(getTagElement(TAG_DISPLAY));
    if (display != 0)
    {
        display->remove(TAG_NAME);
        if (display->isEmpty())
        {
            display->remove(TAG_DISPLAY);
        }
    }
    if (mTag != 0 && mTag->isEmpty())
    {
        mTag = 0;
    }
    setCensorshipInfo(L"", false);
}

bool ItemInstance::hasCustomHoverName()
{
    CompoundTag* display = static_cast<CompoundTag*>(getTagElement(TAG_DISPLAY));
    return display != 0 && display->contains(TAG_NAME, Tag::TAG_STRING);
}

boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*>* ItemInstance::getAttributeModifiers(const EquipmentSlot* slot)
{
    if (!hasTag() || !mTag->contains(L"AttributeModifiers", Tag::TAG_LIST))
    {
        return getItem()->getDefaultAttributeModifiers(slot);
    }

    boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*>* modifiers =
        new boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*>();
    ListTag* entries = mTag->getList(L"AttributeModifiers", Tag::TAG_COMPOUND);
    for (int i = 0; i < entries->size(); ++i)
    {
        CompoundTag* entry = entries->getCompound(i);
        AttributeModifier* modifier = SharedMonsterAttributes::loadAttributeModifier(entry);
        if (modifier == 0)
        {
            continue;
        }
        if (entry->contains(L"Slot", Tag::TAG_STRING) && entry->getString(L"Slot") != slot->getName())
        {
            continue;
        }
        modifiers->emplace(static_cast<eATTRIBUTE_ID>(entry->getInt(L"ID")), modifier);
    }
    return modifiers;
}

std::vector<HtmlString>* ItemInstance::getTooltipLines(boost::shared_ptr<Player> player, bool advanced)
{
    std::vector<HtmlString>* lines = new std::vector<HtmlString>();
    HtmlString title(getHoverName(), eMinecraftColour_NOTSET, false, false, 0);
    if (hasCustomHoverName())
        title.mItalic = true;

    if (gApp.DebugArtToolsOn())
        advanced = true;
    if (advanced)
    {
        std::wstring suffix = L"";
        if (!title.mText.empty())
        {
            title.mText.append(L" (");
            suffix = L")";
        }
        std::wstringstream id;
        id << L"#" << std::setw(4) << std::setfill(L'0') << Item::getId(mItem);
        if (isStackedByData())
            id << L"/" << mAuxValue;
        id << suffix;
        title.mText.append(id.str());
    }
    else if (!hasCustomHoverName() && mItem == Items::filledMap)
    {
        title.mText.append(L" #" + _toString<int>(mAuxValue));
    }
    lines->push_back(title);

    int hideFlags = 0;
    if (hasTag() && mTag->contains(TAG_HIDE_FLAGS, Tag::TAG_ANY_NUMERIC))
        hideFlags = mTag->getInt(TAG_HIDE_FLAGS);

    if ((hideFlags & 0x20) == 0)
        getItem()->appendHoverText(shared_from_this(), player, lines, advanced);

    if (hasTag() && (hideFlags & 0x01) == 0)
    {
        ListTag* enchantments = getEnchantmentTags();
        if (enchantments != NULL && !enchantments->isEmpty())
        {
            for (int index = 0; index < enchantments->size(); ++index)
            {
                const int enchantmentId = enchantments->getCompound(index)->getShort(TAG_ENCH_ID);
                const int level = enchantments->getCompound(index)->getShort(TAG_ENCH_LEVEL);
                if (Enchantment::byId(enchantmentId) != NULL)
                    lines->push_back(Enchantment::byId(enchantmentId)->getFullname(level));
            }
        }
    }

    if ((hideFlags & 0x02) == 0)
    {
        for (int index = 0; index < 6; ++index)
        {
            boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*>* modifiers = getAttributeModifiers(
                EquipmentSlot::SLOTS[index]);
            if (!modifiers->empty())
            {
                lines->push_back(HtmlString(L"", eMinecraftColour_NOTSET, false, false, 0));
                for (boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*>::iterator modifier = modifiers->begin();
                     modifier != modifiers->end(); ++modifier)
                {
                    if (modifier->first < eATTRIBUTE_ID_AttackSpeed || modifier->first == eATTRIBUTE_ID_Armor)
                        lines->push_back(modifier->second->getHoverText(modifier->first, shared_from_this(), player));
                }
            }
            delete modifiers;
        }
    }

    if (hasTag() && getTag()->getBoolean(TAG_UNBREAKABLE) && (hideFlags & 0x04) == 0)
        lines->push_back(
            HtmlString(L10N::GetString(IDS_ITEM_UNBREAKABLE), eMinecraftColour_HTMLColor_9, false, false, 0));

    if (hasTag() && mTag->contains(TAG_CAN_DESTROY, Tag::TAG_LIST) && (hideFlags & 0x08) == 0)
    {
        ListTag* blocks = mTag->getList(TAG_CAN_DESTROY, Tag::TAG_STRING);
        if (blocks->size() > 0)
        {
            lines->push_back(HtmlString(L"", eMinecraftColour_NOTSET, false, false, 0));
            lines->push_back(
                HtmlString(L10N::GetString(IDS_ITEM_CANBREAK), eMinecraftColour_HTMLColor_dark_7, false, false, 0));
            for (int index = 0; index < blocks->size(); ++index)
            {
                Block* block = Block::byString(blocks->getString(index));
                if (block != NULL)
                    lines->push_back(HtmlString(block->getName(), eMinecraftColour_HTMLColor_dark_7, false, false, 0));
                else
                    lines->push_back(HtmlString(L"missingno", eMinecraftColour_HTMLColor_dark_7, false, false, 0));
            }
        }
    }

    if (hasTag() && mTag->contains(TAG_CAN_PLACE_ON, Tag::TAG_LIST) && (hideFlags & 0x10) == 0)
    {
        ListTag* blocks = mTag->getList(TAG_CAN_PLACE_ON, Tag::TAG_STRING);
        if (blocks->size() > 0)
        {
            lines->push_back(HtmlString(L"", eMinecraftColour_NOTSET, false, false, 0));
            lines->push_back(
                HtmlString(L10N::GetString(IDS_ITEM_CANPLACE), eMinecraftColour_HTMLColor_7, false, false, 0));
            for (int index = 0; index < blocks->size(); ++index)
            {
                Block* block = Block::byString(blocks->getString(index));
                if (block != NULL)
                    lines->push_back(HtmlString(block->getName(), eMinecraftColour_HTMLColor_dark_7, false, false, 0));
                else
                    lines->push_back(HtmlString(L"missingno", eMinecraftColour_HTMLColor_dark_7, false, false, 0));
            }
        }
    }

    if (advanced)
    {
        if (isDamaged())
        {
            const std::wstring durability = L"Durability: LOCALISE " + _toString<int>(getMaxDamage() - getDamageValue())
                                            + L" / " + _toString<int>(getMaxDamage());
            lines->push_back(HtmlString(durability, eMinecraftColour_NOTSET, false, false, 0));
        }
        ResourceLocation key = Item::itemRegistry.getKey(getItem());
        lines->push_back(HtmlString(key.toString(), eMinecraftColour_HTMLColor_dark_7, false, false, 0));
        if (hasTag())
        {
            lines->push_back(
                HtmlString(L"NBT: " + _toString<unsigned int>(getTag()->getAllTagsMap()->size()) + L" tag(s)",
                           eMinecraftColour_HTMLColor_dark_7, false, false, 0));
        }
    }
    return lines;
}

std::vector<HtmlString>* ItemInstance::getHoverTextOnly(boost::shared_ptr<Player> player, bool advanced)
{
    std::vector<HtmlString>* lines = new std::vector<HtmlString>();
    getItem()->appendHoverText(not_null_ptr<ItemInstance>(shared_from_this()), player, lines, advanced);

    ListTag* enchantments = hasTag() ? getEnchantmentTags() : 0;
    if (enchantments != 0)
    {
        for (int i = 0; i < enchantments->size(); ++i)
        {
            const int enchantmentId = enchantments->getCompound(i)->getShort(TAG_ENCH_ID);
            const int level = enchantments->getCompound(i)->getShort(TAG_ENCH_LEVEL);
            if (Enchantment::byId(enchantmentId) != NULL)
            {
                lines->push_back(Enchantment::byId(enchantmentId)->getFullname(level));
            }
        }
    }
    return lines;
}

bool ItemInstance::hasFoil()
{
    return getItem()->isFoil(not_null_ptr<ItemInstance>(shared_from_this()));
}

const Rarity* ItemInstance::getRarity()
{
    return getItem()->getRarity(not_null_ptr<ItemInstance>(shared_from_this()));
}

bool ItemInstance::isEnchanted()
{
    if (mTag != 0 && mTag->contains(TAG_ENCH, Tag::TAG_LIST))
    {
        ListTag* list = mTag->getList(TAG_ENCH, Tag::TAG_COMPOUND);
        return !list->isEmpty();
    }
    return false;
}

bool ItemInstance::isEnchantable()
{
    bool enchantable = getItem()->isEnchantable(not_null_ptr<ItemInstance>(shared_from_this()));
    return enchantable && !isEnchanted();
}

void ItemInstance::enchant(Enchantment* enchantment, int level)
{
    if (mTag == 0)
        setTag(new CompoundTag());
    if (!mTag->contains(L"ench"))
        mTag->put(L"ench", new ListTag());

    ListTag* enchantments = mTag->getList(TAG_ENCH, Tag::TAG_COMPOUND);
    CompoundTag* entry = new CompoundTag();
    entry->putShort(TAG_ENCH_ID, static_cast<short>(Enchantment::getId(enchantment)));
    entry->putShort(TAG_ENCH_LEVEL, static_cast<unsigned char>(level));
    enchantments->add(entry);
}

bool ItemInstance::mayBePlacedInAdventureMode()
{
    return getItem()->mayBePlacedInAdventureMode();
}

bool ItemInstance::isFramed()
{
    return mFrame.use_count() > 0;
}

void ItemInstance::setFramed(boost::shared_ptr<ItemFrame> frame)
{
    mFrame = frame;
}

boost::shared_ptr<ItemFrame> ItemInstance::getFrame()
{
    if (mEmptyCache != 0)
    {
        return boost::shared_ptr<ItemFrame>();
    }
    return mFrame.lock();
}

int ItemInstance::getBaseRepairCost()
{
    if (hasTag() && mTag->contains(TAG_REPAIR_COST, Tag::TAG_INT))
        return mTag->getInt(TAG_REPAIR_COST);
    return 0;
}

void ItemInstance::setRepairCost(int cost)
{
    if (!hasTag())
        mTag = new CompoundTag();
    mTag->putInt(TAG_REPAIR_COST, cost);
}

bool ItemInstance::hasAdventureModeBreakTagForBlock(Block* block)
{
    if (block == mBreakBlock)
    {
        return mCanBreak != 0;
    }
    mBreakBlock = block;
    if (!canDestroySpecial(block->defaultBlockState()))
    {
        if (getDestroySpeed(block->defaultBlockState()) <= 1.0f)
        {
            mCanBreak = 0;
            return false;
        }
    }
    mCanBreak = 1;
    return true;
}

bool ItemInstance::hasAdventureModePlaceTagForBlock(Block* block)
{
    if (block != mPlaceBlock)
    {
        mPlaceBlock = block;
        mCanPlace = mayBePlacedInAdventureMode();
    }
    return mCanPlace != 0;
}

void ItemInstance::setPopTime(int time)
{
    mPopTime = time;
}

void ItemInstance::set4JData(int data)
{
    if (mTag == 0)
    {
        if (data == 0)
            return;
        setTag(new CompoundTag());
    }
    if (mTag->contains(L"4jdata"))
    {
        IntTag* tag4j = static_cast<IntTag*>(mTag->get(L"4jdata"));
        tag4j->mValue = data;
    }
    else if (data != 0)
    {
        mTag->put(L"4jdata", new IntTag(data));
    }
}

int ItemInstance::get4JData()
{
    if (mTag == 0 || !mTag->contains(L"4jdata"))
        return 0;
    return static_cast<IntTag*>(mTag->get(L"4jdata"))->mValue;
}

bool ItemInstance::hasPotionStrengthBar()
{
    return mItem == Items::potion && mAuxValue != 0;
}

int ItemInstance::GetPotionStrength()
{
    const unsigned int auxValue = static_cast<unsigned int>(mAuxValue);
    return ((auxValue & 0xf) != 0xc && (auxValue & 0xf) != 5) ? static_cast<int>((auxValue >> 5) & 3)
                                                              : static_cast<int>((auxValue >> 5) & 1);
}

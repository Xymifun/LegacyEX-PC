#ifndef GAME_ITEM_ITEMINSTANCE_HPP
#define GAME_ITEM_ITEMINSTANCE_HPP

#include <string>
#include <vector>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/unordered_map.hpp>
#include "game/core/Attribute.hpp"
#include "game/core/HtmlString.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResult.hpp"
#include "game/core/InteractionResultHolder.hpp"
#include "game/core/CensoredString.hpp"

class Block;
class BlockPos;
class BlockState;
class CompoundTag;
class DataFixerUpper;
class Direction;
class Enchantment;
class EquipmentSlot;
class Item;
class Level;
class ListTag;
class Random;
class Rarity;
class Tag;
class Player;
class LivingEntity;
class Entity;
class ItemFrame;
class MapItem;
class AttributeModifier;

class ItemInstance : public boost::enable_shared_from_this<ItemInstance>
{
    friend class AbstractContainerMenu;
    friend class ItemRenderer;

public:
    explicit ItemInstance(Block* block);
    ItemInstance(Block* block, int count);
    ItemInstance(Block* block, int count, int auxValue);
    explicit ItemInstance(Item* item);
    ItemInstance(Item* item, int count);
    ItemInstance(Item* item, int count, int auxValue);
    explicit ItemInstance(CompoundTag* tag);
    ItemInstance(MapItem* item, int count);
    ItemInstance(const ItemInstance& other);
    ~ItemInstance();
    ItemInstance& operator=(const ItemInstance& other);
    bool isEmpty() const;
    int getCount();
    void updateEmptyCacheFlag();
    void setCount(int count);
    Item* getItem() const;
    void _init(Item* item, int count, int auxValue);
    static void addDataWalkers(DataFixerUpper* fixer);
    not_null_ptr<ItemInstance> copy() const;
    void grow(int amount);
    void shrink(int amount);
    not_null_ptr<ItemInstance> split(int amount);
    InteractionResult::EInteractionResult useOn(boost::shared_ptr<Player> player, Level* level, const BlockPos& pos,
                                                InteractionHand::EInteractionHand hand, const Direction* direction,
                                                float x, float y, float z, bool flag);
    float getDestroySpeed(const BlockState* state);
    bool TestUse(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand);
    InteractionResultHolder use(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand);
    not_null_ptr<ItemInstance> finishUsingItem(Level* level, boost::shared_ptr<Player> player);
    CompoundTag* save(CompoundTag* tag);
    int getMaxStackSize();
    bool hasTag();
    CompoundTag* getTag();
    bool isDamageableItem();
    bool isDamaged();
    bool isStackable();
    bool isStackedByData();
    int getDamageValue();
    int getAuxValue() const;
    void setAuxValue(int auxValue);
    int getMaxDamage();
    bool hurt(int amount, Random* random);
    void hurtAndBreak(int amount, boost::shared_ptr<LivingEntity> entity);
    void hurtEnemy(boost::shared_ptr<LivingEntity> entity, boost::shared_ptr<Player> player);
    void mineBlock(Level* level, const BlockState* state, const BlockPos& pos, boost::shared_ptr<Player> player);
    bool canDestroySpecial(const BlockState* state);
    bool interactEnemy(boost::shared_ptr<Player> player, boost::shared_ptr<LivingEntity> entity,
                       InteractionHand::EInteractionHand hand);
    ItemInstance* copy_not_shared() const;
    static bool tagMatches(const not_null_ptr<ItemInstance>& first, const not_null_ptr<ItemInstance>& second);
    bool matches(const not_null_ptr<ItemInstance>& other, bool compareAuxValue);
    static bool matches(const not_null_ptr<ItemInstance>& first, const not_null_ptr<ItemInstance>& second,
                        bool compareAuxValue);
    bool sameItem(const not_null_ptr<ItemInstance>& other);
    static bool isSame(const not_null_ptr<ItemInstance>& first, const not_null_ptr<ItemInstance>& second);
    bool sameItemIgnoreDurability(const not_null_ptr<ItemInstance>& other);
    static bool isSameIgnoreDurability(const not_null_ptr<ItemInstance>& first,
                                       const not_null_ptr<ItemInstance>& second);
    bool sameItemWithTags(const not_null_ptr<ItemInstance>& other, std::vector<std::wstring>* tags);
    bool sameItem_not_shared(ItemInstance* other);
    int getUseDescriptionId();
    int getDescriptionId(int auxValue);
    void setDescriptionId(int descriptionId);
    std::wstring toString();
    void inventoryTick(Level* level, boost::shared_ptr<Entity> entity, int slot, bool selected);
    void onCraftedBy(Level* level, boost::shared_ptr<Player> player, int amount);
    int getUseDuration();
    int getUseAnimation() const;
    void releaseUsing(Level* level, boost::shared_ptr<LivingEntity> entity, int duration);
    void setTag(CompoundTag* tag);
    void addTagElement(std::wstring name, Tag* tag);
    Tag* getOrCreateTagElement(const std::wstring& name);
    Tag* getTagElement(const std::wstring& name);
    ListTag* getEnchantmentTags();
    bool wasCreatedByRestrictedPlayer();
    std::wstring getHoverNameUnfiltered();
    std::wstring getHoverName();
    not_null_ptr<ItemInstance> setTranslatableName(int name);
    not_null_ptr<ItemInstance> setHoverName(const std::wstring& name);
    void setCensorshipInfo(const std::wstring& name, bool censored);
    std::wstring getOwnerUUID();
    bool wasCreatedOnHost();
    void resetHoverName();
    bool hasCustomHoverName();
    boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*>* getAttributeModifiers(const EquipmentSlot* slot);
    std::vector<HtmlString>* getTooltipLines(boost::shared_ptr<Player> player, bool advanced);
    std::vector<HtmlString>* getHoverTextOnly(boost::shared_ptr<Player> player, bool advanced);
    bool hasFoil();
    const Rarity* getRarity();
    bool isEnchanted();
    bool isEnchantable();
    void enchant(Enchantment* enchantment, int level);
    bool mayBePlacedInAdventureMode();
    bool isFramed();
    void setFramed(boost::shared_ptr<ItemFrame> frame);
    boost::shared_ptr<ItemFrame> getFrame();
    int getBaseRepairCost();
    void setRepairCost(int cost);
    bool hasAdventureModeBreakTagForBlock(Block* block);
    bool hasAdventureModePlaceTagForBlock(Block* block);
    int getPopTime() const;
    void setPopTime(int time);
    void set4JData(int data);
    int get4JData();
    bool hasPotionStrengthBar();
    int GetPotionStrength();
    bool equals(not_null_ptr<ItemInstance> other);
    void removeTagElement(const std::wstring& name);
    void addAttributeModifier(const std::wstring& name, AttributeModifier* modifier, const EquipmentSlot* slot);
    std::wstring getDisplayName();
    static const wchar_t* TAG_ENCH;
    static const wchar_t* TAG_ENCH_ID;
    static const wchar_t* TAG_ENCH_LEVEL;
    static const wchar_t* TAG_DISPLAY;
    static const wchar_t* TAG_NAME;
    static const wchar_t* TAG_LOC_NAME;
    static const wchar_t* TAG_UNBREAKABLE;
    static const wchar_t* TAG_REPAIR_COST;
    static const wchar_t* TAG_OWNER_ID;
    static const wchar_t* TAG_CREATED_ON_HOST;
    static const wchar_t* TAG_CREATED_BY_RESTRICTED;
    static const wchar_t* TAG_FISH_BUCKET;
    static const wchar_t* TAG_CAN_DESTROY;
    static const wchar_t* TAG_CAN_PLACE_ON;
    static const wchar_t* TAG_HIDE_FLAGS;
    static const not_null_ptr<ItemInstance> EMPTY;

private:
    int mCount;
    int mPopTime;
    Item* mItem;
    CompoundTag* mTag;
    unsigned char mEmptyCache;
    int mAuxValue;
    unsigned char mUnknown;
    boost::weak_ptr<ItemFrame> mFrame;
    boost::shared_ptr<CensoredString> mCensorshipInfo;
    Block* mBreakBlock;
    unsigned char mCanBreak;
    Block* mPlaceBlock;
    unsigned char mCanPlace;
};

#endif

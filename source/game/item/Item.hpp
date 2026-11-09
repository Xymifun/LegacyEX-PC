#ifndef GAME_ITEM_ITEM_HPP
#define GAME_ITEM_ITEM_HPP

#include <string>
#include <vector>

#include <boost/functional/hash.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/core/Attribute.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResultHolder.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/MappedRegistry.hpp"
#include "game/world/block/SimpleRegistry.hpp"

class AttributeModifier;
class Block;
class BlockPos;
class BlockState;
class CompoundTag;
class Direction;
class Random;
class Entity;
class HitResult;
class IconRegister;
class Item;
class ItemInstance;
class ItemPropertyFunction;
class ItemTooltipDataHolder;
class Level;
class LivingEntity;
class Material;
class Player;
class Rarity;
class TextureAtlasSprite;

enum UseAnim
{
    UseAnim_none,
    UseAnim_eat,
    UseAnim_drink,
    UseAnim_block,
    UseAnim_bow,
    UseAnim_spear
};

class Item
{
public:
    class Tier
    {
    public:
        Tier(int level, int uses, float speed, float attackDamageBonus, int enchantmentValue, int ordinalValue);

        int getUses() const;
        float getAttackDamageBonus() const;
        float getSpeed() const;
        int getLevel() const;
        int getEnchantmentValue() const;
        Item* getTierItem() const;
        int ordinal() const;

        static const Tier* WOOD;
        static const Tier* STONE;
        static const Tier* IRON;
        static const Tier* DIAMOND;
        static const Tier* GOLD;

    private:
        int mLevel;
        int mUses;
        float mSpeed;
        float mAttackDamageBonus;
        int mEnchantmentValue;
        int mOrdinal;
    };

    static MappedRegistry<ResourceLocation, Item*, boost::hash<ResourceLocation>, std::equal_to<ResourceLocation>,
                          boost::hash<Item*>, std::equal_to<Item*> >
        itemRegistry;

    static boost::unordered_map<Block*, Item*> BLOCK_TO_ITEM;

    Item();

    virtual not_null_ptr<ItemInstance> getDefaultInstance();
    virtual bool verifyTagAfterLoad(CompoundTag* tag);
    virtual unsigned int GetUseTooltip(const ItemTooltipDataHolder& tooltipData);
    virtual ~Item() {}
    virtual InteractionResult::EInteractionResult useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction* direction, float hitX, float hitY, float hitZ,
                                                        bool simulate);
    virtual float getDestroySpeed(not_null_ptr<ItemInstance> itemInstance, const BlockState* state);
    virtual bool TestUse(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand);
    virtual InteractionResultHolder use(Level* level, boost::shared_ptr<Player> player,
                                        InteractionHand::EInteractionHand hand);
    virtual not_null_ptr<ItemInstance> finishUsingItem(not_null_ptr<ItemInstance> itemInstance, Level* level,
                                                       boost::shared_ptr<LivingEntity> entity);
    virtual int getMaxStackSize();
    virtual int getLevelDataForAuxValue(int auxValue);
    virtual bool hurtEnemy(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<LivingEntity> target,
                           boost::shared_ptr<LivingEntity> attacker);
    virtual bool mineBlock(not_null_ptr<ItemInstance> itemInstance, Level* level, const BlockState* state,
                           const BlockPos& pos, boost::shared_ptr<LivingEntity> entity);
    virtual int getAttackDamage(boost::shared_ptr<Entity> entity);
    virtual bool canDestroySpecial(const BlockState* state);
    virtual bool interactEnemy(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                               boost::shared_ptr<LivingEntity> target, InteractionHand::EInteractionHand hand);
    virtual bool isHandEquipped();
    virtual bool isMirroredArt();
    virtual int getDescriptionId(int auxValue);
    virtual int getDescriptionId(not_null_ptr<ItemInstance> itemInstance);
    virtual int getUseDescriptionId();
    virtual int getUseDescriptionId(not_null_ptr<ItemInstance> itemInstance);
    virtual bool shouldOverrideMultiplayerNBT();
    virtual int getColor(not_null_ptr<ItemInstance> itemInstance, int layer);
    virtual void inventoryTick(not_null_ptr<ItemInstance> itemInstance, Level* level, boost::shared_ptr<Entity> entity,
                               int slot, bool selected);
    virtual void onCraftedBy(not_null_ptr<ItemInstance> itemInstance, Level* level, boost::shared_ptr<Player> player);
    virtual bool isComplex();
    virtual int getUseAnimation(not_null_ptr<const ItemInstance> itemInstance) const;
    virtual int getUseDuration(not_null_ptr<ItemInstance> itemInstance);
    virtual void releaseUsing(not_null_ptr<ItemInstance> itemInstance, Level* level,
                              boost::shared_ptr<LivingEntity> entity, int timeLeft);
    virtual void appendHoverText(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                                 std::vector<HtmlString>* lines, bool advanced);
    virtual std::wstring getName(not_null_ptr<ItemInstance> itemInstance);
    virtual bool isFoil(not_null_ptr<ItemInstance> itemInstance);
    virtual const Rarity* getRarity(not_null_ptr<ItemInstance> itemInstance);
    virtual bool isEnchantable(not_null_ptr<ItemInstance> itemInstance);
    virtual int getEnchantmentValue();
    virtual bool mayBePlacedInAdventureMode();
    virtual bool isValidRepairItem(not_null_ptr<ItemInstance> itemInstance, not_null_ptr<ItemInstance> repairItem);
    virtual boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*>* getDefaultAttributeModifiers(
        const EquipmentSlot* slot);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual bool hasMultipleSpriteLayers();
    virtual TextureAtlasSprite* getLayerIcon(int auxValue, int layer, not_null_ptr<ItemInstance> itemInstance);
    virtual int getIconType();
    virtual TextureAtlasSprite* getIcon(int auxValue);
    virtual TextureAtlasSprite* getIcon(not_null_ptr<ItemInstance> itemInstance);
    virtual int GetArmorType();
    virtual int GetOverrideCount();
    virtual int GetOverrideCountColour();
    virtual TextureAtlasSprite* GetOverrideCountIcon(not_null_ptr<ItemInstance> itemInstance);

    static void staticCtor();
    static void staticInit();

    static Item* byBlock(Block* block);
    static Item* byId(int id);
    static Item* byString(const std::wstring& name);
    static int getId(Item* item);
    static int VALIDgetId(Item* item);
    static int getBlockId(Item* item);
    static bool isItemABlock(int id);
    static bool existsWithId(int id);
    static bool IsShulkerBox(int id);
    static bool IsShulkerBox(Item* item);

    static void registerItem(int id, const ResourceLocation& location, Item* item);
    static void registerItem(int id, const std::wstring& name, Item* item);
    static void registerBlock(Block* block, Item* item);
    static void registerBlock(Block* block);

    static int StoneBlockNameResolver(not_null_ptr<ItemInstance> itemInstance);
    static int StoneBlockDescResolver(not_null_ptr<ItemInstance> itemInstance);
    static int DirtBlockNameResolver(not_null_ptr<ItemInstance> itemInstance);
    static int DirtBlockDescResolver(not_null_ptr<ItemInstance> itemInstance);
    static int PlanksBlockNameResolver(not_null_ptr<ItemInstance> itemInstance);
    static int SaplingBlockNameResolver(not_null_ptr<ItemInstance> itemInstance);
    static int SandBlockNameResolver(not_null_ptr<ItemInstance> itemInstance);
    static int SandBlockDescResolver(not_null_ptr<ItemInstance> itemInstance);
    static int Log1BlockNameResolver(not_null_ptr<ItemInstance> itemInstance);
    static int Log2BlockNameResolver(not_null_ptr<ItemInstance> itemInstance);
    static int SpongeBlockNameResolver(not_null_ptr<ItemInstance> itemInstance);
    static int SpongeBlockDescResolver(not_null_ptr<ItemInstance> itemInstance);
    static int SandstoneBlockNameResolver(not_null_ptr<ItemInstance> itemInstance);
    static int WoolBlockNameResolver(not_null_ptr<ItemInstance> itemInstance);
    static int FlowerBlockNameResolver(not_null_ptr<ItemInstance> itemInstance);
    static int FlowerBlockDescResolver(not_null_ptr<ItemInstance> itemInstance);
    static int MonsterEggBlockNameResolver(not_null_ptr<ItemInstance> itemInstance);
    static int StoneBrickBlockNameResolver(not_null_ptr<ItemInstance> itemInstance);
    static int CobblestoneWallBlockNameResolver(not_null_ptr<ItemInstance> itemInstance);
    static int QuartzBlockNameResolver(not_null_ptr<ItemInstance> itemInstance);
    static int ClayBlockNameResolver(not_null_ptr<ItemInstance> itemInstance);
    static int WoolCarpetBlockNameResolver(not_null_ptr<ItemInstance> itemInstance);
    static int DoublePlantBlockNameResolver(not_null_ptr<ItemInstance> itemInstance);
    static int DoublePlantBlockDescResolver(not_null_ptr<ItemInstance> itemInstance);
    static int GlassBlockNameResolver(not_null_ptr<ItemInstance> itemInstance);
    static int GlassPaneBlockNameResolver(not_null_ptr<ItemInstance> itemInstance);
    static int PrismarineBlockNameResolver(not_null_ptr<ItemInstance> itemInstance);
    static int PrismarineBlockDescResolver(not_null_ptr<ItemInstance> itemInstance);
    static int RedSandstoneBlockNameResolver(not_null_ptr<ItemInstance> itemInstance);
    static int ConcreteBlockNameResolver(not_null_ptr<ItemInstance> itemInstance);
    static int ConcretePowderBlockNameResolver(not_null_ptr<ItemInstance> itemInstance);
    static int CoralBlockNameResolver(not_null_ptr<ItemInstance> itemInstance);
    static int CoralBlockDescResolver(not_null_ptr<ItemInstance> itemInstance);
    static int CoralBushNameResolver(not_null_ptr<ItemInstance> itemInstance);
    static int CoralBushDescResolver(not_null_ptr<ItemInstance> itemInstance);

    Item* setDescriptionId(int descriptionId);
    Item* setUseDescriptionId(int useDescriptionId);
    Item* setDescriptionId(int descriptionId, int useDescriptionId);
    Item* setBaseItemTypeAndMaterial(int baseItemType, int baseItemMaterial);
    Item* setIconName(const std::wstring& iconName);
    Item* setMaxStackSize(int maxStackSize);
    Item* setMaxDamage(int maxDamage);
    Item* setAllowOffhand(bool allowOffhand);
    Item* SetPreferInventory(bool preferInventory);
    Item* setCraftingRemainingItem(Item* craftingRemainingItem);
    Item* setStackedByData(bool stackedByData);
    Item* handEquipped();

    void addProperty(ResourceLocation location, const ItemPropertyFunction* function);
    const ItemPropertyFunction* getProperty(ResourceLocation location);
    bool hasProperties();

    Material* getMaterial();
    int getBaseItemType();
    int getMaxDamage();
    bool isStackedByData();
    bool canBeDepleted();
    bool allowOffhand();
    bool PreferInventory();
    bool IsArmor();
    Item* getCraftingRemainingItem();
    bool hasCraftingRemainingItem();
    std::wstring getIconName();
    const wchar_t* getDescription();
    const wchar_t* getDescription(not_null_ptr<ItemInstance> itemInstance);
    HitResult* getPlayerPOVHitResult(Level* level, boost::shared_ptr<Player> player, bool useLiquids);

protected:
    static Random* random;

    int mMaterialData;
    Material* mMaterial;
    SimpleRegistry<ResourceLocation, const ItemPropertyFunction*, boost::hash<ResourceLocation>,
                   std::equal_to<ResourceLocation> >* mProperties;
    int mMaxStackSize;
    int mMaxDamage;
    TextureAtlasSprite* mIcon;
    int mBaseItemType;
    int mBaseItemMaterial;
    bool mHandEquipped;
    bool mStackedByData;
    bool mAllowOffhand;
    Item* mCraftingRemainingItem;
    std::wstring mName;
    int mDescriptionId;
    int mUseDescriptionId;
    std::wstring mIconName;
    bool mPreferInventory;
};

#endif

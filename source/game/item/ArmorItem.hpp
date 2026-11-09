#ifndef GAME_ITEM_ARMORITEM_HPP
#define GAME_ITEM_ARMORITEM_HPP

#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/core/Attribute.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResultHolder.hpp"
#include "game/item/DefaultDispenseItemBehavior.hpp"
#include "game/item/Item.hpp"
#include "game/item/not_null_ptr.hpp"

class AttributeModifier;
class BlockSource;
class IconRegister;
class ItemInstance;
class ItemTooltipDataHolder;
class Level;
class Player;
class SoundEvent;
class TextureAtlasSprite;

class ArmorItem : public Item
{
public:
    class ArmorMaterial
    {
    public:
        static const ArmorMaterial* LEATHER;
        static const ArmorMaterial* CHAIN;
        static const ArmorMaterial* IRON;
        static const ArmorMaterial* GOLD;
        static const ArmorMaterial* DIAMOND;
        static const ArmorMaterial* TURTLE;

        ArmorMaterial(std::wstring name, int durabilityMultiplier, int* slotProtections, int enchantmentValue,
                      const SoundEvent* equipSound, float toughness);
        ~ArmorMaterial();

        int getDefenseForSlot(const EquipmentSlot* slot) const;
        float getToughness() const;
        int getHealthForSlot(const EquipmentSlot* slot) const;
        int getEnchantmentValue() const;
        Item* getTierItem() const;
        const SoundEvent* getEquipSound() const;

    private:
        std::wstring mName;
        int mDurabilityMultiplier;
        int* mSlotProtections;
        int mEnchantmentValue;
        const SoundEvent* mEquipSound;
        float mToughness;
    };

    static DispenseItemBehavior* DISPENSE_ITEM_BEHAVIOR;

    ArmorItem(const ArmorMaterial* material, int renderIndex, const EquipmentSlot* slot);

    virtual unsigned int GetUseTooltip(const ItemTooltipDataHolder& tooltipData);
    virtual bool TestUse(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand);
    virtual InteractionResultHolder use(Level* level, boost::shared_ptr<Player> player,
                                        InteractionHand::EInteractionHand hand);
    virtual int getColor(not_null_ptr<ItemInstance> itemInstance, int layer);
    virtual int getEnchantmentValue();
    virtual bool isValidRepairItem(not_null_ptr<ItemInstance> itemInstance, not_null_ptr<ItemInstance> repairItem);
    virtual boost::unordered_map<eATTRIBUTE_ID, AttributeModifier*>* getDefaultAttributeModifiers(
        const EquipmentSlot* slot);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual bool hasMultipleSpriteLayers();
    virtual TextureAtlasSprite* getLayerIcon(int auxValue, int layer, not_null_ptr<ItemInstance> itemInstance);
    virtual int GetArmorType();

    static void staticCtor();

    static not_null_ptr<ItemInstance> dispenseArmor(BlockSource* blockSource, not_null_ptr<ItemInstance> itemInstance,
                                                    DefaultDispenseItemBehavior::eOUTCOME& outcome);
    static bool isDurabilityDrainedArmor(AttributeModifier* modifier);
    static TextureAtlasSprite* getEmptyIcon(int slotIndex);

    const EquipmentSlot* getSlot();
    const ArmorMaterial* getMaterial();

    bool hasCustomColor(not_null_ptr<ItemInstance> itemInstance);
    void clearColor(not_null_ptr<ItemInstance> itemInstance);
    void setColor(not_null_ptr<ItemInstance> itemInstance, int color);

public:
    const EquipmentSlot* mSlot;
    int mDefense;

private:
    float mToughness;

public:
    int mRenderIndex;

private:
    const ArmorMaterial* mMaterial;
    TextureAtlasSprite* mOverlayIcon;
    TextureAtlasSprite* mEmptyIcon;
};

#endif

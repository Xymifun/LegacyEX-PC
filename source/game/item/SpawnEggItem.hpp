#ifndef GAME_ITEM_SPAWNEGGITEM_HPP
#define GAME_ITEM_SPAWNEGGITEM_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResultHolder.hpp"
#include "game/item/Item.hpp"

class BlockPos;
class Direction;
class IconRegister;
class ItemInstance;
class ItemTooltipDataHolder;
class Level;
class Mob;
class Player;
class ResourceLocation;
class TextureAtlasSprite;
class Entity;

class SpawnEggItem : public Item
{
public:
    SpawnEggItem();

    static ResourceLocation getNamedIdFrom(not_null_ptr<ItemInstance> itemInstance);
    virtual std::wstring getName(not_null_ptr<ItemInstance> itemInstance);
    double getYOffset(Level* level, const BlockPos& pos);
    static boost::shared_ptr<Entity> canSpawn(const ResourceLocation& entityType, Level* level, int* error);
    static boost::shared_ptr<Entity> getMob(Level* level, const ResourceLocation& entityType, double x, double y,
                                            double z, int* error, const std::wstring& customName);
    static boost::shared_ptr<Entity> spawnMobAt(Level* level, const ResourceLocation& entityType, double x, double y,
                                                double z, int* error, const std::wstring& customName);
    static void updateCustomEntityTag(Level* level, boost::shared_ptr<Player> player,
                                      not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Entity> entity);
    static void DisplaySpawnError(boost::shared_ptr<Player> player, int error);
    virtual InteractionResult::EInteractionResult useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction* direction, float hitX, float hitY, float hitZ,
                                                        bool simulate);
    virtual InteractionResultHolder use(Level* level, boost::shared_ptr<Player> player,
                                        InteractionHand::EInteractionHand hand);
    static void addSpawnEntity(not_null_ptr<ItemInstance> itemInstance, const ResourceLocation& entityType);
    virtual bool hasMultipleSpriteLayers();
    virtual TextureAtlasSprite* getLayerIcon(int auxValue, int layer, not_null_ptr<ItemInstance> itemInstance);
    virtual void registerIcons(IconRegister* iconRegister);
    static unsigned long long fjGetType(const not_null_ptr<ItemInstance>& itemInstance);
    virtual bool TestUse(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand);
    virtual unsigned int GetUseTooltip(const ItemTooltipDataHolder& tooltipData);

private:
    TextureAtlasSprite* mOverlayIcon;
};

#endif

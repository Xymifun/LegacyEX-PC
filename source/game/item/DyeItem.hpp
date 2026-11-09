#ifndef GAME_ITEM_DYEITEM_HPP
#define GAME_ITEM_DYEITEM_HPP

#include "game/item/Item.hpp"

class BlockPos;
class Direction;
class IconRegister;
class Level;
class TextureAtlasSprite;

class DyeItem : public Item
{
public:
    static const int DYE_COLORS[16];

    DyeItem();

    virtual int getDescriptionId(not_null_ptr<ItemInstance> itemInstance);
    virtual int getUseDescriptionId(not_null_ptr<ItemInstance> itemInstance);
    static bool growCrop(not_null_ptr<ItemInstance> itemInstance, Level* level, const BlockPos& pos, bool simulate);
    virtual InteractionResult::EInteractionResult useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction* direction, float hitX, float hitY, float hitZ,
                                                        bool simulate);
    static void addGrowthParticles(Level* level, const BlockPos& pos, int count);
    virtual bool interactEnemy(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                               boost::shared_ptr<LivingEntity> target, InteractionHand::EInteractionHand hand);
    virtual TextureAtlasSprite* getIcon(int auxValue);
    virtual void registerIcons(IconRegister* iconRegister);

private:
    TextureAtlasSprite* mIcons[16];
};

#endif

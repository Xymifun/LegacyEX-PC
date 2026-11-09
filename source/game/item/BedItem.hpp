#ifndef GAME_ITEM_BEDITEM_HPP
#define GAME_ITEM_BEDITEM_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResult.hpp"
#include "game/item/Item.hpp"
#include "game/item/not_null_ptr.hpp"

class BlockPos;
class Direction;
class IconRegister;
class ItemInstance;
class Level;
class Player;
class TextureAtlasSprite;

class BedItem : public Item
{
public:
    BedItem();
    virtual InteractionResult::EInteractionResult useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction* direction, float hitX, float hitY, float hitZ,
                                                        bool simulate);
    virtual TextureAtlasSprite* getLayerIcon(int auxValue, int layer, not_null_ptr<ItemInstance> itemInstance);
    virtual bool hasMultipleSpriteLayers();
    virtual void registerIcons(IconRegister* iconRegister);
    virtual int getDescriptionId(not_null_ptr<ItemInstance> itemInstance);

private:
    TextureAtlasSprite* mBedIcon;
};

#endif

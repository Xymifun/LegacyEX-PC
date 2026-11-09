#ifndef GAME_ITEM_FIRECHARGEITEM_HPP
#define GAME_ITEM_FIRECHARGEITEM_HPP

#include "game/item/Item.hpp"

class IconRegister;
class TextureAtlasSprite;

class FireChargeItem : public Item
{
public:
    FireChargeItem();
    virtual InteractionResult::EInteractionResult useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction* direction, float hitX, float hitY, float hitZ,
                                                        bool simulate);
    virtual TextureAtlasSprite* getIcon(int auxValue);
    virtual void registerIcons(IconRegister* iconRegister);

private:
    TextureAtlasSprite* mIcon2;
};

#endif

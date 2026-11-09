#ifndef GAME_ITEM_AIRITEM_HPP
#define GAME_ITEM_AIRITEM_HPP

#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/core/HtmlString.hpp"
#include "game/item/Item.hpp"
#include "game/item/not_null_ptr.hpp"

class Block;
class ItemInstance;
class Player;

class AirItem : public Item
{
public:
    AirItem(Block* block);

    virtual int getDescriptionId(not_null_ptr<ItemInstance> itemInstance);
    virtual void appendHoverText(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                                 std::vector<HtmlString>* lines, bool advanced);

private:
    Block* mBlock;
};

#endif

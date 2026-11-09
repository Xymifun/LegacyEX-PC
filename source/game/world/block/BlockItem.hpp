#ifndef GAME_WORLD_BLOCK_BLOCKITEM_HPP
#define GAME_WORLD_BLOCK_BLOCKITEM_HPP

#include <boost/shared_ptr.hpp>
#include <vector>

#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResult.hpp"
#include "game/core/InteractionResultHolder.hpp"
#include "game/item/Item.hpp"
#include "game/item/not_null_ptr.hpp"

class Block;
class BlockPos;
class Direction;
class IconRegister;
class Level;
class Player;
class ItemInstance;

class BlockItem : public Item
{
public:
    explicit BlockItem(Block* block);
    virtual ~BlockItem() {}

    virtual InteractionResult::EInteractionResult useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction* direction, float hitX, float hitY, float hitZ,
                                                        bool simulate);
    virtual InteractionResultHolder use(Level* level, boost::shared_ptr<Player> player,
                                        InteractionHand::EInteractionHand hand);
    virtual int getDescriptionId(int auxValue);
    virtual int getDescriptionId(not_null_ptr<ItemInstance> itemInstance);
    virtual int getUseDescriptionId();
    virtual int getUseDescriptionId(not_null_ptr<ItemInstance> itemInstance);
    virtual void appendHoverText(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                                 std::vector<HtmlString>* lines, bool advanced);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual int getIconType();
    virtual TextureAtlasSprite* getIcon(int auxValue);
    virtual bool mayPlace(Level* level, const BlockPos& pos, const Direction* direction,
                          boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> itemInstance);

    Block* getBlock();
    BlockItem* setDescriptionId(unsigned int descriptionId);
    static bool updateCustomBlockEntityTag(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos,
                                           not_null_ptr<ItemInstance> itemInstance);
    static boost::shared_ptr<Player> fjGetCurrentlyPlacingBlockPlayer();

    static const std::wstring BLOCK_ENTITY_TAG;

protected:
    Block* mBlock;

private:
    TextureAtlasSprite* mIcon;
};

#endif

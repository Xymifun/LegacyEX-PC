#ifndef GAME_WORLD_BLOCK_DROPPERBLOCK_HPP
#define GAME_WORLD_BLOCK_DROPPERBLOCK_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/block/DispenserBlock.hpp"

class BlockPos;
class BlockTooltipDataHolder;
class IconRegister;
class Level;
class DefaultDispenseItemBehavior;
class DispenseItemBehavior;
template <class T> class not_null_ptr;
class ItemInstance;

class DropperBlock : public DispenserBlock
{
public:
    DropperBlock();
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);

    virtual void registerIcons(IconRegister* iconRegister);
    virtual void dispenseFrom(Level* level, const BlockPos& pos);
    virtual DispenseItemBehavior* getDispenseMethod(not_null_ptr<ItemInstance> itemInstance);
    virtual boost::shared_ptr<BlockEntity> newBlockEntity(Level* level, int data);

private:
    DefaultDispenseItemBehavior* mDropperBehavior;
};

#endif

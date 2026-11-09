#ifndef GAME_WORLD_BLOCK_BEDBLOCKENTITY_HPP
#define GAME_WORLD_BLOCK_BEDBLOCKENTITY_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/block/BlockEntity.hpp"
#include "game/item/not_null_ptr.hpp"

class CompoundTag;
class DyeColor;
class ItemInstance;

class BedBlockEntity : public BlockEntity
{
public:
    BedBlockEntity();

    void setColor(const DyeColor* color);
    void setItemValues(not_null_ptr<ItemInstance> item);
    const DyeColor* getColor();
    not_null_ptr<ItemInstance> getItem();
    virtual boost::shared_ptr<BlockEntity> clone();
    virtual void load(CompoundTag* tag);
    virtual CompoundTag* save(CompoundTag* tag);
    virtual CompoundTag* getUpdateTag();
    virtual boost::shared_ptr<Packet> getUpdatePacket();
    bool isHeadPiece();
    virtual unsigned long long GetType();

    static BlockEntity* create();

private:
    const DyeColor* mColor;
};

#endif

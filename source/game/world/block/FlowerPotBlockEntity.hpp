#ifndef GAME_WORLD_BLOCK_FLOWERPOTBLOCKENTITY_HPP
#define GAME_WORLD_BLOCK_FLOWERPOTBLOCKENTITY_HPP

#include <boost/shared_ptr.hpp>
#include "game/world/block/BlockEntity.hpp"
#include "game/item/not_null_ptr.hpp"

class CompoundTag;
class DataFixerUpper;
class Item;
class ItemInstance;
class Packet;

class FlowerPotBlockEntity : public BlockEntity
{
public:
    FlowerPotBlockEntity(Item* item, int data);
    FlowerPotBlockEntity();

    static BlockEntity* create();
    Item* getPlantItem();
    not_null_ptr<ItemInstance> getPlantItemInstance();
    void setItemInstance(const not_null_ptr<ItemInstance>& item);
    int getItemData();
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual CompoundTag* save(CompoundTag* tag);
    virtual void load(CompoundTag* tag);
    virtual boost::shared_ptr<Packet> getUpdatePacket();
    virtual CompoundTag* getUpdateTag();
    virtual boost::shared_ptr<BlockEntity> clone();
    virtual unsigned long long GetType();

private:
    Item* mPlantItem;
    int mItemData;
};

#endif

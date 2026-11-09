#ifndef GAME_WORLD_BLOCK_COMPARATORBLOCKENTITY_HPP
#define GAME_WORLD_BLOCK_COMPARATORBLOCKENTITY_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/block/BlockEntity.hpp"

class ComparatorBlockEntity : public BlockEntity
{
public:
    static BlockEntity* create();
    virtual unsigned long long GetType();
    virtual void load(CompoundTag* tag);
    virtual CompoundTag* save(CompoundTag* tag);
    virtual boost::shared_ptr<BlockEntity> clone();

    unsigned int getOutputSignal();
    void setOutputSignal(int signal);

private:
    int mOutputSignal;
};

#endif

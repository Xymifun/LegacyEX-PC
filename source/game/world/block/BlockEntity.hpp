#ifndef GAME_WORLD_BLOCK_BLOCKENTITY_HPP
#define GAME_WORLD_BLOCK_BLOCKENTITY_HPP

#include <string>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/BlockPos.hpp"

class Block;
class Level;
class Packet;

class BlockEntity : public boost::enable_shared_from_this<BlockEntity>
{
public:
    typedef BlockEntity* (*Creator)();

    BlockEntity();

    virtual unsigned long long GetType() = 0;
    virtual ~BlockEntity() {}
    virtual Level* getLevel();
    virtual void setLevel(Level* level);
    virtual bool hasLevel();
    virtual void load(CompoundTag* tag);
    virtual CompoundTag* save(CompoundTag* tag);
    virtual void tick();
    virtual void setLevelForLoading(Level* level);
    virtual int getData();
    virtual void setData(int data);
    virtual void setChanged();
    virtual double distanceToSqr(double x, double y, double z);
    virtual double getViewDistance();
    virtual BlockPos getBlockPos();
    virtual Block* getBlock();
    virtual boost::shared_ptr<Packet> getUpdatePacket();
    virtual CompoundTag* getUpdateTag();
    virtual CompoundTag* getUpdateTagForPistonMovement();
    virtual bool isRemoved();
    virtual void setRemoved();
    virtual void clearRemoved();
    virtual bool triggerEvent(int id, int param);
    virtual void clearCache();
    virtual void setPosition(const BlockPos& position);
    virtual bool onlyOpCanSetNbt();
    virtual std::wstring getDisplayName();
    virtual void rotate(Rotation* rotation);
    virtual void mirror(Mirror* mirror);
    virtual boost::shared_ptr<BlockEntity> clone() = 0;

    void setRenderRemoveStage(unsigned char stage);
    void upgradeRenderRemoveStage();
    bool shouldRemoveForRender();
    CompoundTag* saveMetadata(CompoundTag* tag);
    void clone(boost::shared_ptr<BlockEntity> other);
    void operator=(const BlockEntity& other);

    static void staticCtor();
    static void Register(int id, Creator creator, unsigned long long type, std::wstring name);
    static ResourceLocation getKey(unsigned long long type);
    static boost::shared_ptr<BlockEntity> loadStatic(Level* level, CompoundTag* tag);

    bool mRemoved;
    unsigned char mRenderRemoveStage;
    Level* mLevel;
    BlockPos mPos;
    int mData;
    Block* mBlock;
};

#endif

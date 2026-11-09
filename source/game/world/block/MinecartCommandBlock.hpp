#ifndef GAME_WORLD_BLOCK_MINECARTCOMMANDBLOCK_HPP
#define GAME_WORLD_BLOCK_MINECARTCOMMANDBLOCK_HPP

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include "game/core/Minecart.hpp"
#include "game/world/block/BaseCommandBlock.hpp"
#include "game/world/block/EntityDataAccessor.hpp"

class BlockState;
class CompoundTag;
class DataFixerUpper;
class DataOutputStream;
class EntityDataAccessor_Base;
class Level;

class MinecartCommandBlock : public Minecart
{
public:
    class MinecartBaseCommandBlock : public BaseCommandBlock
    {
    public:
        MinecartBaseCommandBlock(const boost::shared_ptr<MinecartCommandBlock>& minecart);
        virtual void onUpdated();
        virtual unsigned int getType();
        virtual void writeCoordinates(DataOutputStream* output);
        virtual BlockPos getCommandSenderBlockPosition();
        virtual Vec3* getCommandSenderWorldPosition();
        virtual Level* getCommandSenderWorld();
        virtual boost::shared_ptr<Entity> getCommandSenderEntity();
        virtual MinecraftServer* getServer();

    private:
        boost::weak_ptr<MinecartCommandBlock> mMinecart;
    };

    MinecartCommandBlock(Level* level);
    MinecartCommandBlock(Level* level, double x, double y, double z);
    static void staticCtor();
    static void addDataWalkers(DataFixerUpper* dataFixerUpper);
    virtual void defineSynchedData();
    BaseCommandBlock* getCommandBlock();
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual const Minecart::Type* getType();
    virtual const BlockState* getDefaultDisplayBlockState();
    virtual void activateMinecart(int x, int y, int z, bool powered);
    virtual bool interact(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual void onSyncedDataUpdated(const EntityDataAccessor_Base* accessor);
    virtual bool onlyOpCanSetNbt();

private:
    static const EntityDataAccessor<std::wstring>* DATA_COMMAND_ID;
    static const EntityDataAccessor<std::wstring>* DATA_LAST_OUTPUT_ID;
    BaseCommandBlock* mCommandBlock;
    int mLastActivatedTick;
};

#endif

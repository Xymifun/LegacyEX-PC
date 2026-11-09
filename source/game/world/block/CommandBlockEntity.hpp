#ifndef GAME_WORLD_BLOCK_COMMANDBLOCKENTITY_HPP
#define GAME_WORLD_BLOCK_COMMANDBLOCKENTITY_HPP

#include <boost/shared_ptr.hpp>
#include "game/core/EntityType.hpp"
#include <string>

#include "game/world/block/BaseCommandBlock.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/CommandBlock.hpp"

class CompoundTag;
class DataOutputStream;

class CommandBlockEntity : public BlockEntity
{
public:
    class AnonymousCommandBlock : public BaseCommandBlock
    {
    public:
        explicit AnonymousCommandBlock(CommandBlockEntity* entity);

        virtual BlockPos getCommandSenderBlockPosition();
        virtual Vec3* getCommandSenderWorldPosition();
        virtual Level* getCommandSenderWorld();
        virtual void setCommand(const std::wstring& command);
        virtual void onUpdated();
        virtual unsigned int getType();
        virtual void writeCoordinates(DataOutputStream* output);

    private:
        CommandBlockEntity* mEntity;
    };

    CommandBlockEntity();
    explicit CommandBlockEntity(CommandBlock::Mode mode);

    static BlockEntity* create();
    BaseCommandBlock* getCommandBlock();
    bool isPowered();
    bool wasConditionMet();
    bool isAutomatic();
    virtual CompoundTag* save(CompoundTag* tag);
    void setAutomatic(bool automatic);
    virtual void load(CompoundTag* tag);
    virtual boost::shared_ptr<Packet> getUpdatePacket();
    virtual bool onlyOpCanSetNbt();
    unsigned int getMode();
    unsigned char isConditional();
    virtual void clearRemoved();
    virtual boost::shared_ptr<BlockEntity> clone();
    void updateNamedPosition(const std::wstring& name, const std::wstring& value);
    virtual unsigned long long GetType();

    CommandStats* getCommandStats();
    void setPowered(bool powered);
    void markConditionMet();

private:
    bool mPowered;
    bool mAutomatic;
    bool mConditionMet;
    BaseCommandBlock* mCommandBlock;
};

#endif

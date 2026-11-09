#ifndef GAME_WORLD_BLOCK_BASECOMMANDBLOCK_HPP
#define GAME_WORLD_BLOCK_BASECOMMANDBLOCK_HPP
#include <string>
#include <boost/shared_ptr.hpp>
#include "game/command/CommandStats.hpp"
#include "game/command/CommandSender.hpp"
class CompoundTag;
class DataOutputStream;
class Entity;
class Level;
class MinecraftServer;
class Player;
class BlockPos;
class Vec3;
class BaseCommandBlock : public CommandSender
{
public:
    BaseCommandBlock();
    virtual ~BaseCommandBlock() {}
    virtual void sendMessage(ClientboundChatPacket::EChatPacketMessage, int*, unsigned int, std::wstring*,
                             unsigned int);
    virtual bool hasPermission(EGameCommand);
    virtual std::wstring getDisplayName();
    virtual BlockPos getCommandSenderBlockPosition() = 0;
    virtual Vec3* getCommandSenderWorldPosition() = 0;
    virtual Level* getCommandSenderWorld() = 0;
    virtual bool isSendMessageEnabled();
    virtual void setCommandStats(CommandStats::CommandStatType, int);
    virtual MinecraftServer* getServer();
    unsigned int getSuccessCount();
    std::wstring getLastOutput();
    CompoundTag* save(CompoundTag*);
    void load(CompoundTag*);
    virtual void setCommand(const std::wstring&);
    std::wstring getCommand();
    void performCommand(Level*);
    std::wstring getName();
    virtual void setName(const std::wstring&, bool);
    void setLastOutput(std::wstring);
    bool usedBy(boost::shared_ptr<Player>);
    virtual void onUpdated() = 0;
    virtual unsigned int getType() = 0;
    virtual void writeCoordinates(DataOutputStream*) = 0;
    void setSuccessCount(int);
    void setTrackOutput(bool);
    bool isTrackOutput();
    CommandStats* getCommandStats();

private:
    int mSuccessCount;
    bool mTrackOutput;
    std::wstring mLastOutput;
    std::wstring mCommand;
    std::wstring mName;
    CommandStats mCommandStats;
};
#endif

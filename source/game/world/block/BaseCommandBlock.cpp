#include "game/world/block/BaseCommandBlock.hpp"
#include "game/data/CompoundTag.hpp"
extern MinecraftServer* gMinecraftServer;
BaseCommandBlock::BaseCommandBlock()
{
    mSuccessCount = 0;
    mTrackOutput = true;
    mLastOutput = L"";
    mCommand = L"";
    mName = L"";
}
unsigned int BaseCommandBlock::getSuccessCount()
{
    return (unsigned int)mSuccessCount;
}
std::wstring BaseCommandBlock::getLastOutput()
{
    return mLastOutput;
}
CompoundTag* BaseCommandBlock::save(CompoundTag* tag)
{
    tag->putString(L"Command", mCommand);
    tag->putInt(L"SuccessCount", mSuccessCount);
    tag->putString(L"CustomName", mName);
    tag->putBoolean(L"TrackOutput", mTrackOutput);
    tag->putString(L"LastOutput", L"");
    tag->putBoolean(L"LPCondionalMode", false);
    tag->putBoolean(L"LPRedstoneMode", false);
    tag->putInt(L"LPCommandMode", 0);
    return tag;
}
void BaseCommandBlock::load(CompoundTag* tag)
{
    mCommand = tag->getString(L"Command");
    mSuccessCount = tag->getInt(L"SuccessCount");
    if (tag->contains(L"CustomName", Tag::TAG_STRING))
        setName(tag->getString(L"CustomName"), true);
    if (tag->contains(L"TrackOutput", Tag::TAG_BYTE))
        mTrackOutput = tag->getBoolean(L"TrackOutput");
    tag->contains(L"LastOutput", Tag::TAG_STRING);
    mLastOutput = L"";
}
bool BaseCommandBlock::hasPermission(EGameCommand)
{
    return false;
}
void BaseCommandBlock::setCommand(const std::wstring& command)
{
    mCommand = command;
    mSuccessCount = 0;
}
std::wstring BaseCommandBlock::getCommand()
{
    return mCommand;
}
void BaseCommandBlock::performCommand(Level*)
{
}
std::wstring BaseCommandBlock::getName()
{
    return mName;
}
std::wstring BaseCommandBlock::getDisplayName()
{
    return getName();
}
void BaseCommandBlock::setName(const std::wstring& name, bool)
{
    mName = name;
}
void BaseCommandBlock::sendMessage(ClientboundChatPacket::EChatPacketMessage, int*, unsigned int, std::wstring*,
                                   unsigned int)
{
}
bool BaseCommandBlock::isSendMessageEnabled()
{
    return false;
}
void BaseCommandBlock::setCommandStats(CommandStats::CommandStatType, int)
{
}
void BaseCommandBlock::setLastOutput(std::wstring output)
{
    mLastOutput = output;
}
bool BaseCommandBlock::usedBy(boost::shared_ptr<Player>)
{
    return false;
}
MinecraftServer* BaseCommandBlock::getServer()
{
    return gMinecraftServer;
}
void BaseCommandBlock::setSuccessCount(int count)
{
    mSuccessCount = count;
}
void BaseCommandBlock::setTrackOutput(bool trackOutput)
{
    mTrackOutput = trackOutput;
}
bool BaseCommandBlock::isTrackOutput()
{
    return mTrackOutput;
}
CommandStats* BaseCommandBlock::getCommandStats()
{
    return &mCommandStats;
}

#ifndef GAME_COMMAND_COMMANDSTATS_HPP
#define GAME_COMMAND_COMMANDSTATS_HPP
#include <string>
#include <boost/shared_ptr.hpp>
class CompoundTag;
class MinecraftServer;
class CommandSender;
class CommandStats
{
public:
    enum CommandStatType
    {
        SUCCESS_COUNT = 0,
        AFFECTED_BLOCKS = 1,
        AFFECTED_ENTITIES = 2,
        AFFECTED_ITEMS = 3,
        QUERY_RESULT = 4
    };
    CommandStats();
    ~CommandStats();
    void setCommandStats(MinecraftServer*, boost::shared_ptr<CommandSender>, CommandStatType, int);
    void setCommandStats(boost::shared_ptr<CommandSender>, CommandStatType, int) {}
    static void unsetStatsSetting(CommandStats*, CommandStatType);
    static void assignStatsSetting(CommandStats*, CommandStatType, std::wstring*, std::wstring*);
    void read(CompoundTag*);
    void write(CompoundTag*);
    void cloneFrom(CommandStats*);
    bool hasStats(CommandStatType);

private:
    std::wstring* m_entityNames;
    std::wstring* m_objectiveNames;
};
#endif

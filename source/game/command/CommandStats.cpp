#include "game/command/CommandStats.hpp"
#include "game/command/CommandSender.hpp"
#include "game/data/CompoundTag.hpp"

std::wstring sStatPrefixes[5] = {L"SuccessCount", L"AffectedBlocks", L"AffectedEntities", L"AffectedItems",
                                 L"QueryResult"};
static std::wstring sStatNames[5];

CommandStats::CommandStats() : m_entityNames(0), m_objectiveNames(0)
{
    int i;
    for (i = 0; i < 5; ++i)
        sStatNames[i] = std::wstring();
}
CommandStats::~CommandStats()
{
}

void CommandStats::setCommandStats(MinecraftServer*, boost::shared_ptr<CommandSender> sender, CommandStatType type,
                                   int value)
{
    setCommandStats(sender, type, value);
}

void CommandStats::unsetStatsSetting(CommandStats* stats, CommandStatType type)
{
    if (stats == 0 || type < SUCCESS_COUNT || type > QUERY_RESULT)
        return;
    if (stats->m_objectiveNames != 0)
        stats->m_objectiveNames[type].clear();
    if (stats->m_entityNames != 0)
        stats->m_entityNames[type].clear();
}

void CommandStats::assignStatsSetting(CommandStats* stats, CommandStatType type, std::wstring* objective,
                                      std::wstring* name)
{
    if (stats == 0 || type < SUCCESS_COUNT || type > QUERY_RESULT)
        return;
    if (stats->m_objectiveNames == 0)
        stats->m_objectiveNames = new std::wstring[5];
    if (stats->m_entityNames == 0)
        stats->m_entityNames = new std::wstring[5];
    if (objective != 0)
        stats->m_objectiveNames[type] = *objective;
    if (name != 0)
        stats->m_entityNames[type] = *name;
}

void CommandStats::read(CompoundTag* tag)
{
    if (tag == 0)
        return;
    CompoundTag* statsTag = tag->getCompound(L"CommandStats");
    if (statsTag == 0)
        return;
    int i;
    for (i = 0; i < 5; ++i)
    {
        std::wstring nameKey = sStatPrefixes[i] + L"Name";
        std::wstring objectiveKey = sStatPrefixes[i] + L"Objective";
        if (statsTag->contains(nameKey) && statsTag->contains(objectiveKey))
        {
            std::wstring name = statsTag->getString(nameKey);
            std::wstring objective = statsTag->getString(objectiveKey);
            assignStatsSetting(this, static_cast<CommandStatType>(i), &objective, &name);
        }
    }
}

void CommandStats::write(CompoundTag* tag)
{
    CompoundTag* result = new CompoundTag();
    int i;
    for (i = 0; i < 5; ++i)
    {
        std::wstring name = m_entityNames[i];
        std::wstring objective = m_objectiveNames[i];
        if (name == sStatPrefixes[i] + L"Name" || objective == sStatPrefixes[i] + L"Objective")
            continue;
        result->putString(sStatPrefixes[i] + L"Name", name);
        result->putString(sStatPrefixes[i] + L"Objective", objective);
    }
    if (!result->isEmpty())
        tag->put(L"CommandStats", result);
}

void CommandStats::cloneFrom(CommandStats* stats)
{
    if (stats == 0)
        return;
    int i;
    for (i = 0; i < 5; ++i)
    {
        if (stats->m_objectiveNames != 0 && stats->m_entityNames != 0)
            assignStatsSetting(this, static_cast<CommandStatType>(i), &stats->m_objectiveNames[i],
                               &stats->m_entityNames[i]);
    }
}

bool CommandStats::hasStats(CommandStatType type)
{
    return type >= SUCCESS_COUNT && type <= QUERY_RESULT && m_objectiveNames != 0 && m_entityNames != 0
           && !m_objectiveNames[type].empty() && !m_entityNames[type].empty();
}

#include "game/core/GeneralStat.hpp"

GeneralStat::GeneralStat(int id, const std::wstring& name, StatFormatter* formatter) : Stat(id, name, formatter)
{
}

GeneralStat::GeneralStat(int id, const std::wstring& name) : Stat(id, name)
{
}

Stat* GeneralStat::postConstruct(bool addToList)
{
    Stat::postConstruct(addToList);
    return this;
}

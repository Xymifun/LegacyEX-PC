#ifndef GAME_CORE_GENERALSTAT_HPP
#define GAME_CORE_GENERALSTAT_HPP

#include <string>

#include "game/core/Stat.hpp"
#include "game/core/StatFormatter.hpp"

class GeneralStat : public Stat
{
public:
    GeneralStat(int id, const std::wstring& name, StatFormatter* formatter);
    GeneralStat(int id, const std::wstring& name);

    virtual Stat* postConstruct(bool addToList);
};

#endif

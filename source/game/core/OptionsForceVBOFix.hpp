#ifndef GAME_CORE_OPTIONSFORCEVBOFIX_HPP
#define GAME_CORE_OPTIONSFORCEVBOFIX_HPP

#include "game/data/DataFix.hpp"

class OptionsForceVBOFix : public DataFix
{
public:
    virtual ~OptionsForceVBOFix();
    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* tag);
};

#endif

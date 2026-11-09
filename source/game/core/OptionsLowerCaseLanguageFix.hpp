#ifndef GAME_CORE_OPTIONSLOWERCASELANGUAGEFIX_HPP
#define GAME_CORE_OPTIONSLOWERCASELANGUAGEFIX_HPP

#include "game/data/CompoundTag.hpp"
#include "game/data/DataFix.hpp"

class OptionsLowerCaseLanguageFix : public DataFix
{
public:
    virtual ~OptionsLowerCaseLanguageFix();
    virtual int getVersion();
    virtual CompoundTag* fix(CompoundTag* tag);
};

#endif

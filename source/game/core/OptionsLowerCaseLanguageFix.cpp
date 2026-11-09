#include "game/core/OptionsLowerCaseLanguageFix.hpp"

OptionsLowerCaseLanguageFix::~OptionsLowerCaseLanguageFix()
{
}

int OptionsLowerCaseLanguageFix::getVersion()
{
    return 816;
}

CompoundTag* OptionsLowerCaseLanguageFix::fix(CompoundTag* tag)
{
    return tag;
}

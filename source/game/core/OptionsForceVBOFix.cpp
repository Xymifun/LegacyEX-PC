#include "game/core/OptionsForceVBOFix.hpp"

OptionsForceVBOFix::~OptionsForceVBOFix()
{
}

int OptionsForceVBOFix::getVersion()
{
    return 505;
}

CompoundTag* OptionsForceVBOFix::fix(CompoundTag* tag)
{
    tag->putString(L"useVbo", L"true");
    return tag;
}

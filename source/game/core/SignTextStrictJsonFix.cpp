#include "game/core/SignTextStrictJsonFix.hpp"

#include "game/data/CompoundTag.hpp"

SignTextStrictJsonFix::~SignTextStrictJsonFix()
{
}

int SignTextStrictJsonFix::getVersion()
{
    return 101;
}

CompoundTag* SignTextStrictJsonFix::fix(CompoundTag* tag)
{
    return tag;
}

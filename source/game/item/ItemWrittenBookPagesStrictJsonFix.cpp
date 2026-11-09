#include "game/item/ItemWrittenBookPagesStrictJsonFix.hpp"

ItemWrittenBookPagesStrictJsonFix::~ItemWrittenBookPagesStrictJsonFix()
{
}

int ItemWrittenBookPagesStrictJsonFix::getVersion()
{
    return 165;
}

CompoundTag* ItemWrittenBookPagesStrictJsonFix::fix(CompoundTag* tag)
{
    return tag;
}

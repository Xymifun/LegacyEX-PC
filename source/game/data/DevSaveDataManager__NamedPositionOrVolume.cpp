#include "game/data/DevSaveDataManager__NamedPositionOrVolume.hpp"

DevSaveDataManager::NamedPositionOrVolume::NamedPositionOrVolume(std::string name, const BlockPos& position)
{
    mName = name;
    mPositions[0] = position;
    mIsVolume = false;
}

DevSaveDataManager::NamedPositionOrVolume::NamedPositionOrVolume(std::string name, const BlockPos& from,
                                                                 const BlockPos& to)
{
    mName = name;
    mPositions[0] = from;
    mPositions[1] = to;
    mIsVolume = true;
}

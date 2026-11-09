#ifndef GAME_DATA_DEVSAVEDATAMANAGER__NAMEDPOSITIONORVOLUME_HPP
#define GAME_DATA_DEVSAVEDATAMANAGER__NAMEDPOSITIONORVOLUME_HPP

#include <string>

#include "game/world/block/BlockPos.hpp"

class DevSaveDataManager
{
public:
    class NamedPositionOrVolume
    {
    public:
        NamedPositionOrVolume(std::string name, const BlockPos& position);
        NamedPositionOrVolume(std::string name, const BlockPos& from, const BlockPos& to);

        std::string mName;
        bool mIsVolume;
        BlockPos mPositions[2];
    };
};

#endif

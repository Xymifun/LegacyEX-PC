#ifndef GAME_WORLD_GENERATION_PLAYERIO_HPP
#define GAME_WORLD_GENERATION_PLAYERIO_HPP

#include <boost/shared_ptr.hpp>

class CompoundTag;
class Player;

class PlayerIO
{
public:
    virtual ~PlayerIO() {}

    virtual void save(boost::shared_ptr<Player> player) = 0;
    virtual CompoundTag* load(boost::shared_ptr<Player> player) = 0;
    virtual void clearOldPlayerFiles() = 0;
    virtual void saveMapIdLookup() = 0;
    virtual void deleteMapFilesForPlayer(boost::shared_ptr<Player> player) = 0;
    virtual void saveAllCachedData() = 0;
};

#endif

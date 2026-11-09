#ifndef GAME_WORLD_GENERATION_VILLAGESIEGE_HPP
#define GAME_WORLD_GENERATION_VILLAGESIEGE_HPP

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

class BlockPos;
class Level;
class Village;
class Vec3;

class VillageSiege
{
public:
    VillageSiege(Level* level);

    Vec3* findRandomSpawnPos(const BlockPos& pos);
    bool tryToSetupSiege();
    bool trySpawn();
    void tick();

private:
    Level* mLevel;
    bool mHasSetupSiege;
    int mSiegeState;
    int mSiegeCount;
    int mNextSpawnTime;
    boost::weak_ptr<Village> mVillage;
    int mSpawnX;
    int mSpawnY;
    int mSpawnZ;
};

#endif

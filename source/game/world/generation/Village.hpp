#ifndef GAME_WORLD_GENERATION_VILLAGE_HPP
#define GAME_WORLD_GENERATION_VILLAGE_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/core/DoorInfo.hpp"
#include "game/world/block/BlockPos.hpp"

class CompoundTag;
class Level;
class LivingEntity;
class Vec3;

class Village
{
public:
    class Aggressor
    {
    public:
        Aggressor(boost::shared_ptr<LivingEntity> entity, int timeStamp);

        boost::shared_ptr<LivingEntity> mEntity;
        int mTimeStamp;
    };

    Village();
    Village(Level* level);
    ~Village();

    void setLevel(Level* level);
    bool isInside(const BlockPos& pos);
    bool canSpawnAt(const BlockPos& size, const BlockPos& pos);
    Vec3* findRandomSpawnPos(const BlockPos& pos, int xSize, int ySize, int zSize);
    bool isDoor(const BlockPos& pos);
    void calcInfo();
    void updateDoors();
    void updateAggressors();
    void countPopulation();
    void countGolem();
    void tick(int tick);
    const BlockPos* getCenter();
    int getRadius();
    int getDoorCount();
    int getStableAge();
    int getPopulationSize();
    std::vector<boost::shared_ptr<DoorInfo> >* getDoorInfos();
    boost::shared_ptr<DoorInfo> getClosestDoorInfo(const BlockPos& pos);
    boost::shared_ptr<DoorInfo> getBestDoorInfo(const BlockPos& pos);
    boost::shared_ptr<DoorInfo> getDoorInfo(const BlockPos& pos);
    bool hasDoorInfo(const BlockPos& pos);
    void addDoorInfo(boost::shared_ptr<DoorInfo> door);
    bool canRemove();
    void addAggressor(boost::shared_ptr<LivingEntity> entity);
    boost::shared_ptr<LivingEntity> getClosestAggressor(boost::shared_ptr<LivingEntity> entity);
    int getStanding(const std::wstring& name);
    bool isVeryBadStanding(std::wstring name);
    boost::shared_ptr<LivingEntity> getClosestBadStandingPlayer(boost::shared_ptr<LivingEntity> entity);
    int modifyStanding(const std::wstring& name, int amount);
    void readAdditionalSaveData(CompoundTag* tag);
    void addAdditonalSaveData(CompoundTag* tag);
    void resetNoBreedTimer();
    bool isBreedTimerOk();
    void rewardAllPlayers(int amount);
    bool isGoodStanding(const std::wstring& name);
    bool isBadStanding(const std::wstring& name);

private:
    Level* mLevel;
    std::vector<boost::shared_ptr<DoorInfo> > mDoors;
    BlockPos mBottom;
    BlockPos mCenter;
    int mRadius;
    int mStable;
    int mTick;
    int mPopulation;
    int mNoBreedTimer;
    boost::unordered_map<std::wstring, int> mPlayerStandings;
    std::vector<Aggressor*> mAggressors;
    int mGolems;
};

#endif

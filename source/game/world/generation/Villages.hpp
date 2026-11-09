#ifndef GAME_WORLD_GENERATION_VILLAGES_HPP
#define GAME_WORLD_GENERATION_VILLAGES_HPP

#include <deque>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/core/DoorInfo.hpp"
#include "game/data/SavedData.hpp"
#include "game/world/block/BlockPos.hpp"

class CompoundTag;
class Dimension;
class Direction;
class Level;
class Village;

class Villages : public SavedData
{
public:
    Villages(const std::wstring& id);
    Villages(Level* level);

    void setLevel(Level* level);
    bool hasQuery(const BlockPos& pos);
    void queryUpdateAround(const BlockPos& pos);
    void removeVillages();
    bool isDoor(const BlockPos& pos);
    boost::shared_ptr<DoorInfo> getDoorInfo(const BlockPos& pos);
    int skyVisibleBlockCount(const BlockPos& pos, const Direction* direction, int count);
    void createDoorInfo(const BlockPos& pos);
    void addDoorInfos(const BlockPos& pos);
    void processNextQuery();
    boost::shared_ptr<Village> getClosestVillage(const BlockPos& pos, int distance);
    void cluster();
    void tick();
    std::vector<boost::shared_ptr<Village> >* getVillages();

    virtual void load(CompoundTag* tag);
    virtual CompoundTag* save(CompoundTag* tag);
    static std::wstring getFileId(Dimension* dimension);

private:
    Level* mLevel;
    std::deque<BlockPos> mToUpdate;
    std::vector<boost::shared_ptr<DoorInfo> > mDoorInfos;
    std::vector<boost::shared_ptr<Village> > mVillages;
    int mTick;
};

#endif

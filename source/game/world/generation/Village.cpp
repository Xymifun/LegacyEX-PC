#include "game/world/generation/Village.hpp"

#include <algorithm>
#include <climits>
#include <cmath>
#include <typeinfo>

#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DoorBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/generation/Villager.hpp"
#include "game/world/generation/VillagerGolem.hpp"
#include "game/world/generation/Player.hpp"

Village::Village()
{
    mBottom = BlockPos::ZERO;
    mCenter = BlockPos::ZERO;
    mRadius = 0;
    mStable = 0;
    mTick = 0;
    mPopulation = 0;
    mGolems = 0;
    mNoBreedTimer = 0;
    mLevel = NULL;
}

Village::Village(Level* level)
{
    mBottom = BlockPos::ZERO;
    mCenter = BlockPos::ZERO;
    mRadius = 0;
    mStable = 0;
    mTick = 0;
    mPopulation = 0;
    mGolems = 0;
    mNoBreedTimer = 0;
    mLevel = level;
}

Village::~Village()
{
    for (std::vector<Aggressor*>::iterator it = mAggressors.begin(); it != mAggressors.end(); ++it)
        delete *it;
}

Village::Aggressor::Aggressor(boost::shared_ptr<LivingEntity> entity, int timeStamp)
{
    mEntity = entity;
    mTimeStamp = timeStamp;
}

void Village::setLevel(Level* level)
{
    mLevel = level;
}
int Village::getDoorCount()
{
    return (int)mDoors.size();
}

bool Village::isInside(const BlockPos& pos)
{
    int radius = mRadius;
    return (double)mCenter.distSqr(pos) < (double)(radius * radius);
}

bool Village::canSpawnAt(const BlockPos& size, const BlockPos& pos)
{
    const BlockState* state = mLevel->getBlockState(pos.below());
    if (!state->isTopSolidBlocking() && state->getBlock() != Blocks::grass_path)
        return false;
    int x = pos.x - size.x / 2;
    int halfZ = size.z / 2;
    int z = pos.z - halfZ;
    MutableBlockPos current(0, 0, 0);
    for (int ix = x; ix < x + size.x; ++ix)
    {
        current.setX(ix);
        for (int iy = pos.y; iy < pos.y + size.y; ++iy)
        {
            current.setY(iy);
            for (int iz = z; iz < z + size.z; ++iz)
            {
                current.setZ(iz);
                const BlockState* currentState = mLevel->getBlockState(current);
                if (currentState->isSolidBlockingCubeAndNotSignalSource())
                    return false;
                if (currentState->getBlock() == Blocks::grass_path)
                    return false;
            }
        }
    }
    return true;
}

Vec3* Village::findRandomSpawnPos(const BlockPos& pos, int xSize, int ySize, int zSize)
{
    BlockPos size(xSize, ySize, zSize);
    for (int i = 0; i < 10; ++i)
    {
        BlockPos candidate = pos.offset(mLevel->mRandom->nextInt(16) - 8, mLevel->mRandom->nextInt(6) - 3,
                                        mLevel->mRandom->nextInt(16) - 8);
        if (isInside(candidate) && canSpawnAt(size, candidate))
            return Vec3::newPermanent(candidate.x, candidate.y, candidate.z);
    }
    return NULL;
}

bool Village::isDoor(const BlockPos& pos)
{
    const BlockState* state = mLevel->getBlockState(pos);
    if (DoorBlock::isDoor(state->getBlock()))
        return state->getMaterial() == Material::WOOD;
    return false;
}

void Village::calcInfo()
{
    int count = (int)mDoors.size();
    if (count == 0)
    {
        mCenter = BlockPos::ZERO;
        mRadius = 0;
    }
    else
    {
        mCenter = BlockPos(mBottom.x / count, mBottom.y / count, mBottom.z / count);
        int maxDistance = 0;
        for (std::vector<boost::shared_ptr<DoorInfo> >::iterator it = mDoors.begin(); it != mDoors.end(); ++it)
        {
            boost::shared_ptr<DoorInfo> door = *it;
            int distance = door->distanceToSqr(mCenter);
            maxDistance = std::max(maxDistance, distance);
        }
        int radius = (int)sqrtf((float)maxDistance) + 1;
        if (radius < 32)
            radius = 32;
        mRadius = radius;
    }
}

void Village::updateDoors()
{
    bool changed = false;
    bool resetBookings = mLevel->mRandom->nextInt(50) == 0;
    std::vector<boost::shared_ptr<DoorInfo> >::iterator it = mDoors.begin();
    while (it != mDoors.end())
    {
        boost::shared_ptr<DoorInfo> door = *it;
        if (resetBookings)
            door->resetBookingCount();
        if (!isDoor(door->getPos()) || std::abs(mTick - door->getTimeStamp()) > 1200)
        {
            mBottom = mBottom.offset(door->getPos().multiply(-1));
            changed = true;
            door->setRemoved(true);
            it = mDoors.erase(it);
        }
        else
            ++it;
    }
    if (changed)
        calcInfo();
}

void Village::updateAggressors()
{
    std::vector<Aggressor*>::iterator it = mAggressors.begin();
    while (it != mAggressors.end())
    {
        Aggressor* aggressor = *it;
        if (!aggressor->mEntity->isAlive() || std::abs(mTick - aggressor->mTimeStamp) > 300)
        {
            delete *it;
            it = mAggressors.erase(it);
        }
        else
            ++it;
    }
}

void Village::countPopulation()
{
    AABB* box = AABB::newTemp(mCenter.x - mRadius, mCenter.y - 4, mCenter.z - mRadius, mCenter.x + mRadius,
                              mCenter.y + 4, mCenter.z + mRadius);
    std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntitiesOfClass(typeid(Villager), box);
    mPopulation = (int)entities->size();
    delete entities;
    if (mPopulation == 0)
        mPlayerStandings.clear();
}

void Village::countGolem()
{
    AABB* box = AABB::newTemp(mCenter.x - mRadius, mCenter.y - 4, mCenter.z - mRadius, mCenter.x + mRadius,
                              mCenter.y + 4, mCenter.z + mRadius);
    std::vector<boost::shared_ptr<Entity> >* entities = mLevel->getEntitiesOfClass(typeid(VillagerGolem), box);
    mGolems = (int)entities->size();
    delete entities;
}

void Village::tick(int tick)
{
    mTick = tick;
    updateDoors();
    updateAggressors();
    if (tick % 20 == 0)
        countPopulation();
    if (tick % 30 == 0)
        countGolem();
    if (mGolems < mPopulation / 10 && mDoors.size() > 20 && mLevel->mRandom->nextInt(7000) == 0)
    {
        Vec3* pos = findRandomSpawnPos(mCenter, 2, 4, 2);
        if (pos != NULL)
        {
            boost::shared_ptr<Entity> golem(new VillagerGolem(mLevel));
            golem->setPos(pos->x, pos->y, pos->z);
            mLevel->addEntity(golem);
            ++mGolems;
        }
    }
}

const BlockPos* Village::getCenter()
{
    return &mCenter;
}
int Village::getRadius()
{
    return mRadius;
}
int Village::getStableAge()
{
    return mTick - mStable;
}
int Village::getPopulationSize()
{
    return mPopulation;
}
std::vector<boost::shared_ptr<DoorInfo> >* Village::getDoorInfos()
{
    return &mDoors;
}

boost::shared_ptr<DoorInfo> Village::getClosestDoorInfo(const BlockPos& pos)
{
    boost::shared_ptr<DoorInfo> result;
    int distance = INT_MAX;
    for (std::vector<boost::shared_ptr<DoorInfo> >::iterator it = mDoors.begin(); it != mDoors.end(); ++it)
    {
        boost::shared_ptr<DoorInfo> door = *it;
        int current = door->distanceToSqr(pos);
        if (current < distance)
        {
            result = door;
            distance = current;
        }
    }
    return result;
}

boost::shared_ptr<DoorInfo> Village::getBestDoorInfo(const BlockPos& pos)
{
    boost::shared_ptr<DoorInfo> result;
    int score = INT_MAX;
    for (std::vector<boost::shared_ptr<DoorInfo> >::iterator it = mDoors.begin(); it != mDoors.end(); ++it)
    {
        boost::shared_ptr<DoorInfo> door = *it;
        int current = door->distanceToSqr(pos);
        if (current > 256)
            current *= 1000;
        else
            current = door->getBookingsCount();
        if (current < score)
        {
            BlockPos doorPos = door->getPos();
            const Direction* direction = door->getDirection();
            if (mLevel->getBlockState(doorPos.relative(direction, 1))
                    ->getBlock()
                    ->isPathfindable(mLevel, doorPos.relative(direction, 1))
                && mLevel->getBlockState(doorPos.relative(direction, -1))
                       ->getBlock()
                       ->isPathfindable(mLevel, doorPos.relative(direction, -1))
                && mLevel->getBlockState(doorPos.above().relative(direction, 1))
                       ->getBlock()
                       ->isPathfindable(mLevel, doorPos.above().relative(direction, 1))
                && mLevel->getBlockState(doorPos.above().relative(direction, -1))
                       ->getBlock()
                       ->isPathfindable(mLevel, doorPos.above().relative(direction, -1)))
            {
                result = door;
                score = current;
            }
        }
    }
    return result;
}

boost::shared_ptr<DoorInfo> Village::getDoorInfo(const BlockPos& pos)
{
    int radius = mRadius;
    if ((double)mCenter.distSqr(pos) > (double)(radius * radius))
        return boost::shared_ptr<DoorInfo>();
    for (std::vector<boost::shared_ptr<DoorInfo> >::iterator it = mDoors.begin(); it != mDoors.end(); ++it)
    {
        boost::shared_ptr<DoorInfo> door = *it;
        if (door->getPos().x == pos.x && door->getPos().z == pos.z && std::abs(door->getPos().y - pos.y) <= 1)
            return boost::shared_ptr<DoorInfo>(door);
    }
    return boost::shared_ptr<DoorInfo>();
}

bool Village::hasDoorInfo(const BlockPos& pos)
{
    return getDoorInfo(pos).get() != NULL;
}

void Village::addDoorInfo(boost::shared_ptr<DoorInfo> door)
{
    mDoors.push_back(door);
    mBottom = mBottom.offset(door->getPos());
    calcInfo();
    mStable = door->getTimeStamp();
}

bool Village::canRemove()
{
    return mDoors.empty();
}

void Village::addAggressor(boost::shared_ptr<LivingEntity> entity)
{
    std::vector<Aggressor*>::iterator it;
    for (it = mAggressors.begin(); it != mAggressors.end(); ++it)
    {
        if ((*it)->mEntity == entity)
        {
            (*it)->mTimeStamp = mTick;
            return;
        }
    }
    mAggressors.push_back(new Aggressor(entity, mTick));
}

boost::shared_ptr<LivingEntity> Village::getClosestAggressor(boost::shared_ptr<LivingEntity> entity)
{
    double distance = 1.7976931348623157e+308;
    Aggressor* closest = NULL;
    for (std::vector<Aggressor*>::iterator it = mAggressors.begin(); it != mAggressors.end(); ++it)
    {
        Aggressor* aggressor = *it;
        double current = aggressor->mEntity->distanceToSqr(entity);
        if (current <= distance)
        {
            closest = aggressor;
            distance = current;
        }
    }
    if (closest != NULL)
        return closest->mEntity;
    return boost::shared_ptr<LivingEntity>();
}

int Village::getStanding(const std::wstring& name)
{
    boost::unordered_map<std::wstring, int>::iterator it = mPlayerStandings.find(name);
    if (it != mPlayerStandings.end())
        return it->second;
    return 0;
}

bool Village::isVeryBadStanding(std::wstring name)
{
    return getStanding(name) <= -15;
}

boost::shared_ptr<LivingEntity> Village::getClosestBadStandingPlayer(boost::shared_ptr<LivingEntity> entity)
{
    boost::shared_ptr<LivingEntity> result;
    double distance = 1.7976931348623157e+308;
    boost::unordered_map<std::wstring, int>::iterator it;
    for (it = mPlayerStandings.begin(); it != mPlayerStandings.end(); ++it)
    {
        std::wstring name = it->first;
        if (isVeryBadStanding(name))
        {
            boost::shared_ptr<Player> player = mLevel->getPlayerByName(name);
            if (player)
            {
                double current = player->distanceToSqr(entity);
                if (current > distance)
                    continue;
                result = player;
                distance = current;
            }
        }
    }
    return result;
}

int Village::modifyStanding(const std::wstring& name, int amount)
{
    int value = Mth::clamp(getStanding(name) + amount, -30, 10);
    mPlayerStandings[name] = value;
    return value;
}

void Village::readAdditionalSaveData(CompoundTag* tag)
{
    mPopulation = tag->getInt(L"PopSize");
    mRadius = tag->getInt(L"Radius");
    mGolems = tag->getInt(L"Golems");
    mStable = tag->getInt(L"Stable");
    mTick = tag->getInt(L"Tick");
    mNoBreedTimer = tag->getInt(L"MTick");
    mCenter = BlockPos(tag->getInt(L"CX"), tag->getInt(L"CY"), tag->getInt(L"CZ"));
    mBottom = BlockPos(tag->getInt(L"ACX"), tag->getInt(L"ACY"), tag->getInt(L"ACZ"));
    ListTag* doors = tag->getList(L"Doors", Tag::TAG_COMPOUND);
    int i;
    for (i = 0; i < doors->size(); ++i)
    {
        CompoundTag* door = doors->getCompound(i);
        boost::shared_ptr<DoorInfo> info(
            new DoorInfo(BlockPos(door->getInt(L"X"), door->getInt(L"Y"), door->getInt(L"Z")), door->getInt(L"IDX"),
                         door->getInt(L"IDZ"), door->getInt(L"TS")));
        mDoors.push_back(info);
    }
    ListTag* players = tag->getList(L"Players", Tag::TAG_COMPOUND);
    for (i = 0; i < players->size(); ++i)
    {
        CompoundTag* player = players->getCompound(i);
        mPlayerStandings.insert(std::make_pair(player->getString(L"Name"), player->getInt(L"S")));
    }
}

void Village::addAdditonalSaveData(CompoundTag* tag)
{
    tag->putInt(L"PopSize", mPopulation);
    tag->putInt(L"Radius", mRadius);
    tag->putInt(L"Golems", mGolems);
    tag->putInt(L"Stable", mStable);
    tag->putInt(L"Tick", mTick);
    tag->putInt(L"MTick", mNoBreedTimer);
    tag->putInt(L"CX", mCenter.x);
    tag->putInt(L"CY", mCenter.y);
    tag->putInt(L"CZ", mCenter.z);
    tag->putInt(L"ACX", mBottom.x);
    tag->putInt(L"ACY", mBottom.y);
    tag->putInt(L"ACZ", mBottom.z);
    ListTag* doors = new ListTag();
    std::vector<boost::shared_ptr<DoorInfo> >::iterator it;
    for (it = mDoors.begin(); it != mDoors.end(); ++it)
    {
        boost::shared_ptr<DoorInfo> info = *it;
        CompoundTag* door = new CompoundTag();
        door->putInt(L"X", info->getPos().x);
        door->putInt(L"Y", info->getPos().y);
        door->putInt(L"Z", info->getPos().z);
        door->putInt(L"IDX", info->getInsideDx());
        door->putInt(L"IDZ", info->getInsideDz());
        door->putInt(L"TS", info->getTimeStamp());
        doors->add(door);
    }
    tag->put(L"Doors", doors);
    ListTag* players = new ListTag();
    for (boost::unordered_map<std::wstring, int>::iterator player = mPlayerStandings.begin();
         player != mPlayerStandings.end(); ++player)
    {
        CompoundTag* entry = new CompoundTag();
        entry->putString(L"Name", player->first);
        entry->putInt(L"S", player->second);
        players->add(entry);
    }
    tag->put(L"Players", players);
}

void Village::resetNoBreedTimer()
{
    mNoBreedTimer = mTick;
}
bool Village::isBreedTimerOk()
{
    return mNoBreedTimer == 0 || mTick - mNoBreedTimer >= 3600;
}
void Village::rewardAllPlayers(int amount)
{
    boost::unordered_map<std::wstring, int>::iterator it;
    for (it = mPlayerStandings.begin(); it != mPlayerStandings.end(); ++it)
        modifyStanding(it->first, amount);
}

#ifndef GAME_ENTITY_MOB_ENTITYTICKCACHE_HPP
#define GAME_ENTITY_MOB_ENTITYTICKCACHE_HPP

class AABB;
class BlockPos;
class BlockState;
class Level;

class EntityTickCache
{
public:
    EntityTickCache();
    ~EntityTickCache();

    int getIndexInCache(int x, int y, int z) const;
    void blockChanged(const BlockPos& pos);
    void updateExtents(int x, int y, int z);
    unsigned short getElement(int x, int y, int z);
    const BlockState* getBlockState(int x, int y, int z);
    void clearCache();
    void buildCache(Level* level, int minX, int minY, int minZ, int maxX, int maxY, int maxZ);
    void buildCache(Level* level, AABB* box);

    int mMinX;
    int mMinY;
    int mMinZ;
    int mMaxX;
    int mMaxY;
    int mMaxZ;
    int mAccessedMinX;
    int mAccessedMinY;
    int mAccessedMinZ;
    int mAccessedMaxX;
    int mAccessedMaxY;
    int mAccessedMaxZ;
    int mLastCenterX;
    int mLastCenterY;
    int mLastCenterZ;
    unsigned short* mElements;
    const BlockState** mBlockStates;
    unsigned int mLastFrameUpdated;
    bool mCacheChanged;
    Level* mLevel;
};

#endif

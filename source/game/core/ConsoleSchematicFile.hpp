#ifndef GAME_CORE_CONSOLESCHEMATICFILE_HPP
#define GAME_CORE_CONSOLESCHEMATICFILE_HPP

#include <string>
#include <utility>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/core/Compression.hpp"
#include "game/core/Direction.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DevSaveDataManager__NamedPositionOrVolume.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BlockPos.hpp"

class AABB;
class BlockTransaction;
class DataInputStream;
class DataOutputStream;
class Level;
class LevelChunk;
class ListTag;
class Painting;
class Vec3;

class ConsoleSchematicFile
{
public:
    enum ESchematicRotation
    {
        eSchematicRotation_0,
        eSchematicRotation_90,
        eSchematicRotation_180,
        eSchematicRotation_270
    };
    enum ESchematicMirror
    {
        eSchematicMirror_None,
        eSchematicMirror_LeftRight,
        eSchematicMirror_FrontBack,
        eSchematicMirror_Both
    };

    class PasteData
    {
    public:
        Level* mLevel;
        AABB* mArea;
        ESchematicRotation mRotation;
        ESchematicMirror mMirror;
        int mMinChunkX;
        int mMaxChunkX;
        int mCurrentChunkX;
        int mMinChunkZ;
        int mMaxChunkZ;
        int mCurrentChunkZ;
        int mWidth;
        int mHeight;
        int mDepth;
        arrayWithLength<unsigned short> mElements;
        arrayWithLength<unsigned short> mExtraElements;
        arrayWithLength<unsigned char> mData;
        arrayWithLength<unsigned char> mExtraData;
        std::vector<std::pair<BlockPos*, CompoundTag*> > mBlockEntities;
        std::vector<std::pair<Vec3*, CompoundTag*> > mEntities;
        std::vector<DevSaveDataManager::NamedPositionOrVolume> mNamedPositions;
        std::vector<std::pair<int, std::wstring> > mUpdatedNames;
        bool mIncludeAir;
        bool mIncludeSpecial;
        PasteData();
    };

    int mRefCount;
    BlockPos mOrigin;
    int mWidth;
    int mHeight;
    int mDepth;
    std::vector<std::pair<BlockPos*, CompoundTag*> > mBlockEntities;
    std::vector<std::pair<Vec3*, CompoundTag*> > mEntities;
    std::vector<DevSaveDataManager::NamedPositionOrVolume> mNamedPositions;
    arrayWithLength<unsigned short> mElements;
    arrayWithLength<unsigned short> mExtraElements;
    arrayWithLength<unsigned char> mData;
    arrayWithLength<unsigned char> mExtraData;
    bool mCanUndo;
    bool mMobs;
    AABB* mUndoArea;
    arrayWithLength<unsigned short> mUndoElements;
    arrayWithLength<unsigned short> mUndoExtraElements;
    arrayWithLength<unsigned char> mUndoData;
    arrayWithLength<unsigned char> mUndoExtraData;
    std::vector<std::pair<BlockPos*, CompoundTag*> > mUndoBlockEntities;
    std::vector<std::pair<Vec3*, CompoundTag*> > mUndoEntities;
    std::vector<DevSaveDataManager::NamedPositionOrVolume> mUndoNamedPositions;
    int mUndoWidth;
    int mUndoHeight;
    int mUndoDepth;
    AABB* mStoredCopyArea;
    AABB* mStoredUndoArea;
    bool mUndoReady;
    ESchematicRotation mRotation;
    ESchematicMirror mMirror;
    PasteData mPasteData;
    bool mHasCopy;
    bool mHasPaste;
    bool mReserved;

    ConsoleSchematicFile();
    ~ConsoleSchematicFile();
    void chunkCoordToSchematicCoord(AABB*, int, int, ESchematicRotation, ESchematicMirror, int&, int&);
    bool CanPasteBlock(unsigned short, bool, bool);
    unsigned short rotateElementValue(unsigned short, ESchematicRotation);
    void setElements(arrayWithLength<unsigned short>&, arrayWithLength<unsigned short>, ESchematicRotation,
                     ESchematicMirror, int, int, int, int, int, int, int&, bool, bool);
    void pasteIntoArray(arrayWithLength<unsigned char>&, arrayWithLength<unsigned char>,
                        arrayWithLength<unsigned short>, int, int, int, bool, bool, bool);
    void setData(arrayWithLength<unsigned char>&, arrayWithLength<unsigned char>, ESchematicMirror, int, int, int, int,
                 int, int, int&, bool, bool, arrayWithLength<unsigned short>, int&);
    long long writeSchematicIntoArrays(int, int, int, int, int, int, int, int, int, PasteData*,
                                       arrayWithLength<unsigned short>, arrayWithLength<unsigned short>,
                                       arrayWithLength<unsigned char>, arrayWithLength<unsigned char>);
    long long applyBlocksAndData(LevelChunk*, PasteData*);
    long long applyBlocksAndData(LevelChunk*, AABB*, ESchematicRotation, ESchematicMirror);
    void schematicCoordToChunkCoord(AABB*, double, double, ESchematicRotation, ESchematicMirror, double&, double&);
    const Direction* HandleRotation(const Direction*, ESchematicMirror, ESchematicRotation);
    void offsetPainting(Level*, boost::shared_ptr<Painting>, ESchematicRotation, ESchematicMirror);
    void applyBlockEntities(LevelChunk*, AABB*, PasteData*, Level*);
    void applyBlockEntities(LevelChunk*, AABB*, AABB*, ESchematicRotation, ESchematicMirror, Level*);
    long long applyLighting(LevelChunk*, AABB*, AABB*, ESchematicRotation);
    static int calcBlockIdsSizeLegacy(int, int, int);
    static int calcHalfByteDataSizeLegacy(int, int, int);
    static int calcFullDataSizeLegacy(int, int, int);
    static int calculateFullElementSize(int, int, int);
    void load(DataInputStream*);
    static std::wstring formatFilenameWithExt(const std::wstring&);
    void _applySchematicData(Level*, AABB*, ESchematicRotation, arrayWithLength<unsigned short>,
                             arrayWithLength<unsigned short>, arrayWithLength<unsigned char>,
                             arrayWithLength<unsigned char>, std::vector<std::pair<BlockPos*, CompoundTag*> >&,
                             std::vector<std::pair<Vec3*, CompoundTag*> >&,
                             std::vector<DevSaveDataManager::NamedPositionOrVolume>&, int, int, int, bool, bool, bool);
    bool undoPrevious(Level*);
    void getElements(LevelChunk*, arrayWithLength<unsigned short>*, arrayWithLength<unsigned short>*, int, int, int,
                     int, int, int, int&);
    void getData(arrayWithLength<unsigned char>, arrayWithLength<unsigned char>*, int&, int, int, int, int, int, int);
    void fillArraysFromLevel(Level*, int, int, int, int, int, int, arrayWithLength<unsigned short>*,
                             arrayWithLength<unsigned short>*, arrayWithLength<unsigned char>*,
                             arrayWithLength<unsigned char>*);
    static std::vector<boost::shared_ptr<class BlockEntity> >* getBlockEntitiesInRegion(LevelChunk*, int, int, int, int,
                                                                                        int, int);
    void getBlockEntitiesInRegion(Level*, int, int, int, int, int, int,
                                  std::vector<std::pair<BlockPos*, CompoundTag*> >&);
    void getEntitiesInRegion(Level*, int, int, int, int, int, int, std::vector<std::pair<Vec3*, CompoundTag*> >&);
    ListTag* getBlockEntitiesInRegionAsTag(Level*, int, int, int, int, int, int);
    ListTag* getEntitiesInRegionAsTag(Level*, int, int, int, int, int, int);
    bool generateSchematicFile(DataOutputStream*, Level*, int, int, int, int, int, int, Compression::ECompressionTypes);
    void applySchematicFile(BlockTransaction*);
    void save_tags(DataOutputStream*);
    void save(DataOutputStream*);
    void tick();
    void CalculatePoints(Vec3**, Vec3**) const;
    AABB* getCopyAABB();
    AABB* getPasteAABB();
    void resetCopyVolume();
    void setPointA(double, double, double);
    void setPointB(double, double, double);
    void SetPasteOrigin(BlockPos);
    BlockPos GetPasteOrigin();
    void setRotation(ESchematicRotation);
    void setMirror(ESchematicMirror);
    void setMobs(bool);
    Vec3* getPointA();
    Vec3* getPointB();
    ESchematicRotation getRotation();
    ESchematicMirror getMirror();
    void _storeSchematic(Level*, int, int, int, int, int, int, arrayWithLength<unsigned short>&,
                         arrayWithLength<unsigned short>&, arrayWithLength<unsigned char>&,
                         arrayWithLength<unsigned char>&, std::vector<std::pair<BlockPos*, CompoundTag*> >&,
                         std::vector<std::pair<Vec3*, CompoundTag*> >&,
                         std::vector<DevSaveDataManager::NamedPositionOrVolume>&);
    void storeUndoSchematic(Level*, int, int, int, int, int, int);
    bool applySchematicFile(Level*, bool, int, int, int, ESchematicRotation, bool, bool, bool, DataInputStream*);
    bool applySchematicFile(Level*, bool, int, int, int, bool, bool, bool, DataInputStream*);
    void storeLocalUndoBoundingBox();
    void storeLocalBoundingBox();
    void storeTempSchematic(Level*, int, int, int, int, int, int, Compression::ECompressionTypes);
    void storeTempSchematic(Level*, Compression::ECompressionTypes);
    bool generateSchematicFile(DataOutputStream*, Level*, Compression::ECompressionTypes);
    bool isValidCopy();
    void notifyNamedPositionUpdate(int, std::wstring);
};

#endif

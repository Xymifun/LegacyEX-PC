#ifndef GAME_WORLD_GENERATION_STRUCTUREPIECE_HPP
#define GAME_WORLD_GENERATION_STRUCTUREPIECE_HPP

#include <vector>

#include "game/world/block/BlockPos.hpp"

class BlockState;
class BoundingBox;
class ChunkPos;
class CompoundTag;
class DataInputStream;
class DataOutputStream;
class Direction;
class DoorBlock;
class Level;
class Mirror;
class Random;
class ResourceLocation;
class Rotation;
class StructureManager;

enum EStructurePiece
{
    eStructurePiece_MonumentBuilding = 0,
    eStructurePiece_OceanMonumentEntryRoom = 1,
    eStructurePiece_OceanMonumentSimpleRoom = 2,
    eStructurePiece_OceanMonumentSimpleTopRoom = 3,
    eStructurePiece_OceanMonumentDoubleYRoom = 4,
    eStructurePiece_OceanMonumentDoubleXRoom = 5,
    eStructurePiece_OceanMonumentDoubleXYRoom = 6,
    eStructurePiece_OceanMonumentDoubleYZRoom = 7,
    eStructurePiece_OceanMonumentDoubleZRoom = 8,
    eStructurePiece_OceanMonumentCoreRoom = 9,
    eStructurePiece_OceanMonumentWingRoom = 10,
    eStructurePiece_OceanMonumentPenthouse = 11,
    eStructurePiece_MineShaftRoom = 12,
    eStructurePiece_MineShaftCorridor = 13,
    eStructurePiece_MineShaftCrossing = 14,
    eStructurePiece_MineShaftStairs = 15,
    eStructurePiece_BridgeStraight = 16,
    eStructurePiece_BridgeEndFiller = 17,
    eStructurePiece_BridgeCrossing = 18,
    eStructurePiece_RoomCrossing = 19,
    eStructurePiece_StairsRoom = 20,
    eStructurePiece_MonsterThrone = 21,
    eStructurePiece_CastleEntrance = 22,
    eStructurePiece_CastleStalkRoom = 23,
    eStructurePiece_CastleSmallCorridorPiece = 24,
    eStructurePiece_CastleSmallCorridorCrossingPiece = 25,
    eStructurePiece_CastleSmallCorridorRightTurnPiece = 26,
    eStructurePiece_CastleSmallCorridorLeftTurnPiece = 27,
    eStructurePiece_CastleCorridorStairsPiece = 28,
    eStructurePiece_CastleCorridorTBalconyPiece = 29,
    eStructurePiece_NetherBridgeStartPiece = 30,
    eStructurePiece_DesertPyramidPiece = 31,
    eStructurePiece_JunglePyramidPiece = 32,
    eStructurePiece_SwamplandHut = 33,
    eStructurePiece_Igloo = 34,
    eStructurePiece_FillerCorridor = 35,
    eStructurePiece_StairsDown = 36,
    eStructurePiece_Straight = 37,
    eStructurePiece_ChestCorridor = 38,
    eStructurePiece_StraightStairsDown = 39,
    eStructurePiece_LeftTurn = 40,
    eStructurePiece_RightTurn = 41,
    eStructurePiece_StrongholdRoomCrossing = 42,
    eStructurePiece_PrisonHall = 43,
    eStructurePiece_Library = 44,
    eStructurePiece_FiveCrossing = 45,
    eStructurePiece_PortalRoom = 46,
    eStructurePiece_StrongholdStartPiece = 47,
    eStructurePiece_Well = 48,
    eStructurePiece_StraightRoad = 49,
    eStructurePiece_SimpleHouse = 50,
    eStructurePiece_SmallTemple = 51,
    eStructurePiece_BookHouse = 52,
    eStructurePiece_SmallHut = 53,
    eStructurePiece_PigHouse = 54,
    eStructurePiece_TwoRoomHouse = 55,
    eStructurePiece_Smithy = 56,
    eStructurePiece_Farmland = 57,
    eStructurePiece_DoubleFarmland = 58,
    eStructurePiece_LightPost = 59,
    eStructurePiece_VillageStartPiece = 60,
    eStructurePiece_EndCityPiece = 61,
    eStructurePiece_WoodlandMansionPiece = 62,
    eStructurePiece_BuriedTreasurePiece = 63,
    eStructurePiece_ShipwreckPiece = 64,
    eStructurePiece_OceanRuin = 65
};

class StructurePiece
{
public:
    class BlockSelector
    {
    public:
        BlockSelector();

        virtual ~BlockSelector();

        virtual void next(Random& random, int x, int y, int z, bool edge);
        virtual const BlockState* getNext();

    protected:
        const BlockState* mNext;
    };

    StructurePiece();
    StructurePiece(int genDepth);

    virtual EStructurePiece GetType() = 0;

    virtual ~StructurePiece();

    virtual CompoundTag* createTag();
    virtual void addAdditionalSaveData(DataOutputStream* output) = 0;
    virtual void load(Level* level, DataInputStream* input);
    virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* structureManager) = 0;
    virtual void addChildren(StructurePiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random);
    virtual bool postProcess(Level* level, Random& random, BoundingBox* box) = 0;

    virtual BoundingBox* getBoundingBox();
    virtual BlockPos* getLocatorPosition();

    virtual void placeBlock(Level* level, const BlockState* state, int x, int y, int z, BoundingBox* box);
    virtual const BlockState* getBlock(Level* level, int x, int y, int z, BoundingBox* box);
    virtual bool isAboveGround(Level* level, int x, int y, int z);
    virtual int getSkyBrightness(Level* level, int x, int y, int z, BoundingBox* box);

    virtual void generateAirBox(Level* level, BoundingBox* box, int x0, int y0, int z0, int x1, int y1, int z1);
    virtual void generateBox(Level* level, BoundingBox* box, int x0, int y0, int z0, int x1, int y1, int z1,
                             const BlockState* outerState, const BlockState* innerState, bool existingOnly);
    virtual void generateBox(Level* level, BoundingBox* box, BoundingBox* fillBox, const BlockState* outerState,
                             const BlockState* innerState, bool existingOnly);
    virtual void generateBox(Level* level, BoundingBox* box, int x0, int y0, int z0, int x1, int y1, int z1,
                             bool existingOnly, Random& random, BlockSelector* selector);
    virtual void generateBox(Level* level, BoundingBox* box, BoundingBox* fillBox, bool existingOnly, Random& random,
                             BlockSelector* selector);
    virtual void generateMaybeBox(Level* level, BoundingBox* box, Random& random, float chance, int x0, int y0, int z0,
                                  int x1, int y1, int z1, const BlockState* outerState, const BlockState* innerState,
                                  bool existingOnly, bool undergroundOnly);
    virtual void maybeGenerateBlock(Level* level, BoundingBox* box, Random& random, float chance, int x, int y, int z,
                                    const BlockState* state);
    virtual void generateUpperHalfSphere(Level* level, BoundingBox* box, int x0, int y0, int z0, int x1, int y1, int z1,
                                         const BlockState* state, bool excludeAir);
    virtual void generateFullSphere(Level* level, BoundingBox* box, int centreX, int centreY, int centreZ, int radius,
                                    const BlockState* shellState, const BlockState* innerState, bool existingOnly);
    virtual void generateAirColumnUp(Level* level, int x, int y, int z, BoundingBox* box);
    virtual void generateAirBoxUp(Level* level, int x0, int z0, int x1, int z1, int y, BoundingBox* box);
    virtual void fillColumnDown(Level* level, const BlockState* state, int x, int y, int z, BoundingBox* box);
    virtual void fillBoxDown(Level* level, const BlockState* state, int x0, int z0, int x1, int z1, int y,
                             BoundingBox* box);

    virtual bool createChest(Level* level, BoundingBox* box, Random& random, int x, int y, int z,
                             ResourceLocation* lootTable);
    virtual bool createChest(Level* level, BoundingBox* box, Random& random, const BlockPos& pos,
                             ResourceLocation* lootTable, const BlockState* state);
    virtual bool createDispenser(Level* level, BoundingBox* box, Random& random, int x, int y, int z,
                                 const Direction* facing, ResourceLocation* lootTable);

    virtual void addStructurePieceSaveData(DataOutputStream* output, std::vector<bool> firstBools,
                                           std::vector<bool> secondBools, std::vector<int> firstInts,
                                           std::vector<int> secondInts);
    virtual void readStructurePieceSaveData(DataInputStream* input, std::vector<bool>& firstBools,
                                            std::vector<bool>& secondBools, std::vector<int>& firstInts,
                                            std::vector<int>& secondInts);
    virtual void addBoolSaveData(DataOutputStream* output, std::vector<bool> values);
    virtual std::vector<bool> readBoolSaveData(DataInputStream* input);
    virtual void addIntSaveData(DataOutputStream* output, std::vector<int> values);
    virtual std::vector<int> readIntSaveData(DataInputStream* input);

    virtual void move(int x, int y, int z);
    virtual Rotation* getRotation();
    virtual Mirror* getMirror();

    void write(DataOutputStream* output);

    void setBoundingBox(BoundingBox* boundingBox);

    static void fromOrientationSetRotationAndMirror(const Direction* orientation, Rotation*& rotation, Mirror*& mirror);

    void setOrientation(const Direction* orientation);
    const Direction* getOrientation();

    int getGenDepth();
    void setGenDepth(int genDepth);

    static StructurePiece* findCollisionPiece(std::vector<StructurePiece*>* pieces, BoundingBox* box);

    bool edgesLiquid(Level* level, BoundingBox* box);

    int getWorldX(int x, int z);
    int getWorldY(int y);
    int getWorldZ(int x, int z);

    void createDoor(Level* level, BoundingBox* box, Random& random, int x, int y, int z, const Direction* facing,
                    DoorBlock* door);

    void setRotation(Rotation* rotation);

    void setDirty();
    void clearDirty();
    bool isDirty();

    bool isInChunk(ChunkPos* chunkPos);

public:
    BoundingBox* mBoundingBox;

protected:
    const Direction* mOrientation;
    Mirror* mMirror;
    Rotation* mRotation;
    int mGenDepth;
    bool mDirty;
};

#endif

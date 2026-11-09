#ifndef GAME_CORE_STRONGHOLDPIECES_HPP
#define GAME_CORE_STRONGHOLDPIECES_HPP

#include <vector>

#include "game/core/BoundingBox.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/generation/StructureManager.hpp"
#include "game/world/generation/StructurePiece.hpp"

class StrongholdPieces
{
public:
    enum EPieceClass
    {
        ePieceClass_None = 0,
        ePieceClass_Straight = 1,
        ePieceClass_PrisonHall = 2,
        ePieceClass_LeftTurn = 3,
        ePieceClass_RightTurn = 4,
        ePieceClass_RoomCrossing = 5,
        ePieceClass_StraightStairsDown = 6,
        ePieceClass_StairsDown = 7,
        ePieceClass_FiveCrossing = 8,
        ePieceClass_ChestCorridor = 9,
        ePieceClass_Library = 10,
        ePieceClass_PortalRoom = 11
    };

    class PieceWeight
    {
    public:
        PieceWeight(EPieceClass pieceClass, int weight, int maxPlaceCount);
        virtual ~PieceWeight();
        bool isValid();
        virtual bool doPlace(int depth);
        EPieceClass pieceClass;
        int weight;
        int placeCount;
        int maxPlaceCount;
    };

    class PieceWeight_Library : public PieceWeight
    {
    public:
        PieceWeight_Library(EPieceClass pieceClass, int weight, int maxPlaceCount)
            : PieceWeight(pieceClass, weight, maxPlaceCount)
        {
        }
        virtual ~PieceWeight_Library();
        virtual bool doPlace(int depth);
    };

    class PieceWeight_PortalRoom : public PieceWeight
    {
    public:
        PieceWeight_PortalRoom(EPieceClass pieceClass, int weight, int maxPlaceCount)
            : PieceWeight(pieceClass, weight, maxPlaceCount)
        {
        }
        virtual ~PieceWeight_PortalRoom();
        virtual bool doPlace(int depth);
    };

    class StartPiece;

    class StrongholdPiece : public StructurePiece
    {
    public:
        enum SmallDoorType
        {
            OPENING = 0,
            WOOD_DOOR,
            GRATES,
            IRON_DOOR
        };

        StrongholdPiece(int depth);
        StrongholdPiece();
        virtual ~StrongholdPiece() {}
        static bool isOkBox(BoundingBox* box, StartPiece* start);
        SmallDoorType randomSmallDoor(Random& random);
        virtual void addAdditionalSaveData(DataOutputStream* output);
        virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* manager);
        void generateSmallDoor(Level* level, Random& random, BoundingBox* box, SmallDoorType type, int x, int y, int z);
        StructurePiece* generateSmallDoorChildForward(StartPiece* start, std::vector<StructurePiece*>* pieces,
                                                      Random& random, int offsetX, int offsetY);
        StructurePiece* generateSmallDoorChildLeft(StartPiece* start, std::vector<StructurePiece*>* pieces,
                                                   Random& random, int offsetY, int offsetXZ);
        StructurePiece* generateSmallDoorChildRight(StartPiece* start, std::vector<StructurePiece*>* pieces,
                                                    Random& random, int offsetY, int offsetXZ);

    protected:
        SmallDoorType mEntryDoor;
    };

    class ChestCorridor : public StrongholdPiece
    {
    public:
        ChestCorridor();
        ChestCorridor(int depth, Random& random, BoundingBox* box, const Direction* direction);
        static ChestCorridor* createPiece(std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
                                          const Direction* direction, int depth);
        virtual void addAdditionalSaveData(DataOutputStream* output);
        virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* manager);
        virtual void addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces, Random& random);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
        bool mHasPlacedChest;
    };

    class FillerCorridor : public StrongholdPiece
    {
    public:
        FillerCorridor();
        FillerCorridor(int depth, Random& random, BoundingBox* box, const Direction* direction);
        static BoundingBox* findPieceBox(std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
                                         const Direction* direction);
        virtual void addAdditionalSaveData(DataOutputStream* output);
        virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* manager);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
        int mSteps;
    };

    class FiveCrossing : public StrongholdPiece
    {
    public:
        FiveCrossing();
        FiveCrossing(int depth, Random& random, BoundingBox* box, const Direction* direction);
        static FiveCrossing* createPiece(std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
                                         const Direction* direction, int depth);
        virtual void addAdditionalSaveData(DataOutputStream* output);
        virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* manager);
        virtual void addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces, Random& random);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
        bool mLeftLow;
        bool mLeftHigh;
        bool mRightLow;
        bool mRightHigh;
    };

    class LeftTurn : public StrongholdPiece
    {
    public:
        LeftTurn();
        LeftTurn(int depth, Random& random, BoundingBox* box, const Direction* direction);
        static LeftTurn* createPiece(std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
                                     const Direction* direction, int depth);
        virtual void addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces, Random& random);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };

    class Library : public StrongholdPiece
    {
    public:
        Library();
        Library(int depth, Random& random, BoundingBox* box, const Direction* direction);
        static Library* createPiece(std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
                                    const Direction* direction, int depth);
        virtual void addAdditionalSaveData(DataOutputStream* output);
        virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* manager);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
        bool mIsTall;
    };

    class PortalRoom : public StrongholdPiece
    {
    public:
        PortalRoom();
        PortalRoom(int depth, Random& random, BoundingBox* box, const Direction* direction);
        static PortalRoom* createPiece(std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
                                       const Direction* direction, int depth);
        virtual void addAdditionalSaveData(DataOutputStream* output);
        virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* manager);
        virtual void addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces, Random& random);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
        bool mHasPlacedSpawner;
    };

    class PrisonHall : public StrongholdPiece
    {
    public:
        PrisonHall();
        PrisonHall(int depth, Random& random, BoundingBox* box, const Direction* direction);
        static PrisonHall* createPiece(std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
                                       const Direction* direction, int depth);
        virtual void addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces, Random& random);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };

    class RoomCrossing : public StrongholdPiece
    {
    public:
        RoomCrossing();
        RoomCrossing(int depth, Random& random, BoundingBox* box, const Direction* direction);
        static RoomCrossing* createPiece(std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
                                         const Direction* direction, int depth);
        virtual void addAdditionalSaveData(DataOutputStream* output);
        virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* manager);
        virtual void addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces, Random& random);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
        int mType;
    };

    class Straight : public StrongholdPiece
    {
    public:
        Straight();
        Straight(int depth, Random& random, BoundingBox* box, const Direction* direction);
        static Straight* createPiece(std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
                                     const Direction* direction, int depth);
        virtual void addAdditionalSaveData(DataOutputStream* output);
        virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* manager);
        virtual void addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces, Random& random);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
        bool mLeftChild;
        bool mRightChild;
    };

    class StraightStairsDown : public StrongholdPiece
    {
    public:
        StraightStairsDown();
        StraightStairsDown(int depth, Random& random, BoundingBox* box, const Direction* direction);
        static StraightStairsDown* createPiece(std::vector<StructurePiece*>* pieces, Random& random, int x, int y,
                                               int z, const Direction* direction, int depth);
        virtual void addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces, Random& random);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };

    class StairsDown : public StrongholdPiece
    {
    public:
        StairsDown(int depth, Random& random, int x, int z);
        StairsDown();
        StairsDown(int depth, Random& random, BoundingBox* box, const Direction* direction);
        static StairsDown* createPiece(std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
                                       const Direction* direction, int depth);
        virtual void addAdditionalSaveData(DataOutputStream* output);
        virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* manager);
        virtual void addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces, Random& random);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
        bool mIsSource;
    };

    class RightTurn : public LeftTurn
    {
    public:
        RightTurn();
        RightTurn(int depth, Random& random, BoundingBox* box, const Direction* direction);
        virtual void addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces, Random& random);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };

    class StartPiece : public StairsDown
    {
    public:
        StartPiece(int depth, Random& random, int x, int z, Level* level);
        StartPiece();
        virtual BlockPos* getLocatorPosition();
        virtual EStructurePiece GetType();
        bool mStartFlag;
        PieceWeight* mLastPieceWeight;
        PortalRoom* mPortalRoom;
        Level* mLevel;
        std::vector<StructurePiece*> mPendingChildren;
    };

    class SmoothStoneSelector : public StructurePiece::BlockSelector
    {
    public:
        virtual void next(Random& random, int x, int y, int z, bool edge);
    };

    static void resetPieces();
    static void staticCtor();
    static unsigned long long loadStatic();
    static bool updatePieceWeight();
    static StructurePiece* findAndCreatePieceFactory(EPieceClass pieceClass, std::vector<StructurePiece*>* pieces,
                                                     Random& random, int x, int y, int z, const Direction* direction,
                                                     int depth);
    static StructurePiece* generatePieceFromSmallDoor(StartPiece* start, std::vector<StructurePiece*>* pieces,
                                                      Random& random, int x, int y, int z, const Direction* direction,
                                                      int depth);
    static StructurePiece* generateAndAddPiece(StartPiece* start, std::vector<StructurePiece*>* pieces, Random& random,
                                               int x, int y, int z, const Direction* direction, int depth);
};

#endif

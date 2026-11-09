#ifndef GAME_CORE_NETHERBRIDGEPIECES_HPP
#define GAME_CORE_NETHERBRIDGEPIECES_HPP

#include <list>
#include <vector>

#include "game/core/BoundingBox.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/generation/StructureManager.hpp"
#include "game/world/generation/StructurePiece.hpp"

class NetherBridgePieces
{
public:
    enum EPieceClass
    {
        ePieceClass_BridgeStraight = 0,
        ePieceClass_BridgeCrossing = 2,
        ePieceClass_RoomCrossing = 3,
        ePieceClass_StairsRoom = 4,
        ePieceClass_MonsterThrone = 5,
        ePieceClass_CastleEntrance = 6,
        ePieceClass_CastleStalkRoom = 7,
        ePieceClass_CastleSmallCorridorPiece = 8,
        ePieceClass_CastleSmallCorridorCrossingPiece = 9,
        ePieceClass_CastleSmallCorridorRightTurnPiece = 10,
        ePieceClass_CastleSmallCorridorLeftTurnPiece = 11,
        ePieceClass_CastleCorridorStairsPiece = 12,
        ePieceClass_CastleCorridorTBalconyPiece = 13
    };

    class PieceWeight;
    class StartPiece;
    class NetherBridgePiece;
    class BridgeCrossing;
    class BridgeEndFiller;
    class BridgeStraight;
    class CastleCorridorStairsPiece;
    class CastleCorridorTBalconyPiece;
    class CastleEntrance;
    class CastleSmallCorridorCrossingPiece;
    class CastleSmallCorridorLeftTurnPiece;
    class CastleSmallCorridorPiece;
    class CastleSmallCorridorRightTurnPiece;
    class CastleStalkRoom;
    class MonsterThrone;
    class RoomCrossing;
    class StairsRoom;

    static PieceWeight* bridgePieceWeights[6];
    static PieceWeight* castlePieceWeights[7];

    static void loadStatic();
    static StructurePiece* findAndCreateBridgePieceFactory(PieceWeight* weight, std::vector<StructurePiece*>* pieces,
                                                           Random& random, int x, int y, int z,
                                                           const Direction* direction, int depth);

    class NetherBridgePiece : public StructurePiece
    {
    public:
        NetherBridgePiece(int depth);
        NetherBridgePiece();
        virtual ~NetherBridgePiece() {}
        static bool isOkBox(BoundingBox* box, StartPiece* start);
        virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* manager);
        virtual void addAdditionalSaveData(DataOutputStream* output);
        int updatePieceWeight(std::list<PieceWeight*>* weights);
        StructurePiece* generatePiece(StartPiece* start, std::list<PieceWeight*>* weights,
                                      std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
                                      const Direction* direction, int depth);
        StructurePiece* generateAndAddPiece(StartPiece* start, std::vector<StructurePiece*>* pieces, Random& random,
                                            int x, int y, int z, const Direction* direction, int depth, bool isCastle);
        StructurePiece* generateChildForward(StartPiece* start, std::vector<StructurePiece*>* pieces, Random& random,
                                             int offsetX, int offsetY, bool isCastle);
        StructurePiece* generateChildLeft(StartPiece* start, std::vector<StructurePiece*>* pieces, Random& random,
                                          int offsetY, int offsetXZ, bool isCastle);
        StructurePiece* generateChildRight(StartPiece* start, std::vector<StructurePiece*>* pieces, Random& random,
                                           int offsetY, int offsetXZ, bool isCastle);
        void generateLightPost(Level* level, Random& random, BoundingBox* box, int x, int y, int z, int xOffset,
                               int zOffset);
        void generateLightPostFacingRight(Level* level, Random& random, BoundingBox* box, int x, int y, int z);
        void generateLightPostFacingLeft(Level* level, Random& random, BoundingBox* box, int x, int y, int z);
        void generateLightPostFacingUp(Level* level, Random& random, BoundingBox* box, int x, int y, int z);
        void generateLightPostFacingDown(Level* level, Random& random, BoundingBox* box, int x, int y, int z);
    };

    class PieceWeight
    {
    public:
        PieceWeight(EPieceClass pieceClass, int weight, int maxPlaceCount, bool allowInRow);
        PieceWeight(EPieceClass pieceClass, int weight, int maxPlaceCount);
        bool doPlace(int depth);
        bool isValid();
        EPieceClass mPieceClass;
        int mWeight;
        int mPlaceCount;
        int mMaxPlaceCount;
        bool mAllowInRow;
    };

    class BridgeStraight : public NetherBridgePiece
    {
    public:
        BridgeStraight();
        virtual ~BridgeStraight() {}
        BridgeStraight(int depth, Random& random, BoundingBox* box, const Direction* direction);
        static StructurePiece* createPiece(std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
                                           const Direction* direction, int depth);
        virtual void addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces, Random& random);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };

    class CastleCorridorStairsPiece : public NetherBridgePiece
    {
    public:
        CastleCorridorStairsPiece();
        virtual ~CastleCorridorStairsPiece() {}
        CastleCorridorStairsPiece(int depth, Random& random, BoundingBox* box, const Direction* direction);
        static StructurePiece* createPiece(std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
                                           const Direction* direction, int depth);
        virtual void addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces, Random& random);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };

    class CastleCorridorTBalconyPiece : public NetherBridgePiece
    {
    public:
        CastleCorridorTBalconyPiece();
        virtual ~CastleCorridorTBalconyPiece() {}
        CastleCorridorTBalconyPiece(int depth, Random& random, BoundingBox* box, const Direction* direction);
        static StructurePiece* createPiece(std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
                                           const Direction* direction, int depth);
        virtual void addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces, Random& random);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };

    class CastleEntrance : public NetherBridgePiece
    {
    public:
        CastleEntrance();
        virtual ~CastleEntrance() {}
        CastleEntrance(int depth, Random& random, BoundingBox* box, const Direction* direction);
        static StructurePiece* createPiece(std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
                                           const Direction* direction, int depth);
        virtual void addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces, Random& random);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };

    class CastleSmallCorridorCrossingPiece : public NetherBridgePiece
    {
    public:
        CastleSmallCorridorCrossingPiece();
        virtual ~CastleSmallCorridorCrossingPiece() {}
        CastleSmallCorridorCrossingPiece(int depth, Random& random, BoundingBox* box, const Direction* direction);
        static StructurePiece* createPiece(std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
                                           const Direction* direction, int depth);
        virtual void addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces, Random& random);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };

    class CastleSmallCorridorPiece : public NetherBridgePiece
    {
    public:
        CastleSmallCorridorPiece();
        virtual ~CastleSmallCorridorPiece() {}
        CastleSmallCorridorPiece(int depth, Random& random, BoundingBox* box, const Direction* direction);
        static StructurePiece* createPiece(std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
                                           const Direction* direction, int depth);
        virtual void addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces, Random& random);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };

    class CastleStalkRoom : public NetherBridgePiece
    {
    public:
        CastleStalkRoom();
        virtual ~CastleStalkRoom() {}
        CastleStalkRoom(int depth, Random& random, BoundingBox* box, const Direction* direction);
        static StructurePiece* createPiece(std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
                                           const Direction* direction, int depth);
        virtual void addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces, Random& random);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };

    class RoomCrossing : public NetherBridgePiece
    {
    public:
        RoomCrossing();
        virtual ~RoomCrossing() {}
        RoomCrossing(int depth, Random& random, BoundingBox* box, const Direction* direction);
        static StructurePiece* createPiece(std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
                                           const Direction* direction, int depth);
        virtual void addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces, Random& random);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };

    class StairsRoom : public NetherBridgePiece
    {
    public:
        StairsRoom();
        virtual ~StairsRoom() {}
        StairsRoom(int depth, Random& random, BoundingBox* box, const Direction* direction);
        static StructurePiece* createPiece(std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
                                           const Direction* direction, int depth);
        virtual void addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces, Random& random);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };

    class BridgeCrossing : public NetherBridgePiece
    {
    public:
        BridgeCrossing();
        BridgeCrossing(Random& random, int x, int z);
        BridgeCrossing(int depth, Random& random, BoundingBox* box, const Direction* direction);
        virtual ~BridgeCrossing() {}
        static StructurePiece* createPiece(std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
                                           const Direction* direction, int depth);
        virtual void addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces, Random& random);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };

    class BridgeEndFiller : public NetherBridgePiece
    {
    public:
        BridgeEndFiller();
        BridgeEndFiller(int depth, Random& random, BoundingBox* box, const Direction* direction);
        virtual ~BridgeEndFiller() {}
        static StructurePiece* createPiece(std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
                                           const Direction* direction, int depth);
        virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* manager);
        virtual void addAdditionalSaveData(DataOutputStream* output);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
        int mSelfSeed;
    };

    class CastleSmallCorridorLeftTurnPiece : public NetherBridgePiece
    {
    public:
        CastleSmallCorridorLeftTurnPiece();
        CastleSmallCorridorLeftTurnPiece(int depth, Random& random, BoundingBox* box, const Direction* direction);
        virtual ~CastleSmallCorridorLeftTurnPiece() {}
        static StructurePiece* createPiece(std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
                                           const Direction* direction, int depth);
        virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* manager);
        virtual void addAdditionalSaveData(DataOutputStream* output);
        virtual void addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces, Random& random);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
        bool mIsNeedingChest;
    };

    class CastleSmallCorridorRightTurnPiece : public NetherBridgePiece
    {
    public:
        CastleSmallCorridorRightTurnPiece();
        CastleSmallCorridorRightTurnPiece(int depth, Random& random, BoundingBox* box, const Direction* direction);
        virtual ~CastleSmallCorridorRightTurnPiece() {}
        static StructurePiece* createPiece(std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
                                           const Direction* direction, int depth);
        virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* manager);
        virtual void addAdditionalSaveData(DataOutputStream* output);
        virtual void addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces, Random& random);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
        bool mIsNeedingChest;
    };

    class MonsterThrone : public NetherBridgePiece
    {
    public:
        MonsterThrone();
        MonsterThrone(int depth, Random& random, BoundingBox* box, const Direction* direction);
        virtual ~MonsterThrone() {}
        static StructurePiece* createPiece(std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
                                           const Direction* direction, int depth);
        virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* manager);
        virtual void addAdditionalSaveData(DataOutputStream* output);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
        bool mHasPlacedSpawner;
    };

    class StartPiece : public BridgeCrossing
    {
    public:
        StartPiece(Random& random, int x, int z, Level* level);
        StartPiece();
        virtual ~StartPiece() {}
        virtual EStructurePiece GetType();
        PieceWeight* mPreviousPiece;
        Level* mLevel;
        std::list<PieceWeight*> mAvailableBridgePieces;
        std::list<PieceWeight*> mAvailableCastlePieces;
        std::vector<StructurePiece*> mPendingChildren;
    };
};

#endif

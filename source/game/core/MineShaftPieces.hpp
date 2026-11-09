#ifndef GAME_CORE_MINESHAFTPIECES_HPP
#define GAME_CORE_MINESHAFTPIECES_HPP

#include <list>
#include <vector>

#include "game/core/BoundingBox.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/generation/MineShaftFeature.hpp"
#include "game/world/generation/StructureManager.hpp"
#include "game/world/generation/StructurePiece.hpp"
#include "game/world/generation/StructureFeatureIO.hpp"

class MineShaftPieces
{
public:
    class MineShaftPiece : public StructurePiece
    {
    public:
        MineShaftPiece(int genDepth, MineShaftFeature::MineShaftType type);
        MineShaftPiece();
        virtual ~MineShaftPiece() {}

        const BlockState* getPlanksBlock();
        const BlockState* getFenceBlock();
        bool isSupportingBox(Level* level, BoundingBox* box, int x0, int x1, int y, int z);
        virtual void addAdditionalSaveData(DataOutputStream* output);
        virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* structureManager);

    public:
        MineShaftFeature::MineShaftType mType;
    };

    class MineShaftCorridor : public MineShaftPiece
    {
    public:
        MineShaftCorridor();
        MineShaftCorridor(int genDepth, Random& random, BoundingBox* box, const Direction* direction,
                          MineShaftFeature::MineShaftType type);

        static BoundingBox* findCorridorSize(std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
                                             const Direction* direction);
        virtual void addAdditionalSaveData(DataOutputStream* output);
        virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* structureManager);
        virtual void addChildren(StructurePiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random);
        bool createChest(Level* level, BoundingBox* box, Random& random, int x, int y, int z,
                         ResourceLocation* lootTable);
        void placeSupport(Level* level, BoundingBox* box, int x0, int y0, int z, int y1, int x1, Random& random);
        void placeCobWeb(Level* level, BoundingBox* box, Random& random, float chance, int x, int y, int z);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();

    private:
        bool mHasRails;
        bool mHasSpiderWebs;
        bool mHasPlacedSupport;
        int mNumSections;
    };

    class MineShaftCrossing : public MineShaftPiece
    {
    public:
        MineShaftCrossing();
        MineShaftCrossing(int genDepth, Random& random, BoundingBox* box, const Direction* direction,
                          MineShaftFeature::MineShaftType type);

        static BoundingBox* findCrossing(std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
                                         const Direction* direction);
        virtual void addAdditionalSaveData(DataOutputStream* output);
        virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* structureManager);
        virtual void addChildren(StructurePiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random);
        void placeSupportPillar(Level* level, BoundingBox* box, int x, int y0, int z, int y1);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();

    private:
        const Direction* mDirection;
        bool mIsTwoFloored;
    };

    class MineShaftRoom : public MineShaftPiece
    {
    public:
        MineShaftRoom(int genDepth, Random& random, int x, int z, MineShaftFeature::MineShaftType type);
        MineShaftRoom();
        virtual ~MineShaftRoom();
        virtual void addChildren(StructurePiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random);
        virtual void move(int x, int y, int z);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        virtual void addAdditionalSaveData(DataOutputStream* output);
        virtual void readAdditionalSaveData(DataInputStream* input, StructureManager* structureManager);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();

    private:
        std::list<BoundingBox*> mChildPieces;
    };

    class MineShaftStairs : public MineShaftPiece
    {
    public:
        MineShaftStairs();
        MineShaftStairs(int genDepth, Random& random, BoundingBox* box, const Direction* direction,
                        MineShaftFeature::MineShaftType type);
        static BoundingBox* findStairs(std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z,
                                       const Direction* direction);
        virtual void addChildren(StructurePiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random);
        virtual bool postProcess(Level* level, Random& random, BoundingBox* box);
        static StructurePiece* Create();
        virtual EStructurePiece GetType();
    };

    static void loadStatic();
    static StructurePiece* createRandomShaftPiece(std::vector<StructurePiece*>* pieces, Random& random, int x, int y,
                                                  int z, const Direction* direction, int genDepth,
                                                  MineShaftFeature::MineShaftType type);
    static StructurePiece* generateAndAddPiece(StructurePiece* startPiece, std::vector<StructurePiece*>* pieces,
                                               Random& random, int x, int y, int z, const Direction* direction,
                                               int genDepth);
    static void staticCtor();
};

#endif

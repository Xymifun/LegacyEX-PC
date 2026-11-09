#include "game/core/NetherBridgePieces.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>
#include <cstdlib>

#include "game/core/BuiltInLootTables.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/MobSpawnerBlockEntity.hpp"
#include "game/world/block/StairBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/StructureFeatureIO.hpp"

NetherBridgePieces::PieceWeight* NetherBridgePieces::bridgePieceWeights[6] = {
    new PieceWeight(ePieceClass_BridgeStraight, 30, 0, true), new PieceWeight(ePieceClass_BridgeCrossing, 10, 4),
    new PieceWeight(ePieceClass_MonsterThrone, 15, 2),        new PieceWeight(ePieceClass_CastleEntrance, 15, 1),
    new PieceWeight(ePieceClass_RoomCrossing, 10, 4),         new PieceWeight(ePieceClass_StairsRoom, 10, 3),
};

NetherBridgePieces::PieceWeight* NetherBridgePieces::castlePieceWeights[7] = {
    new PieceWeight(ePieceClass_CastleStalkRoom, 30, 2),
    new PieceWeight(ePieceClass_CastleSmallCorridorPiece, 25, 0, true),
    new PieceWeight(ePieceClass_CastleSmallCorridorCrossingPiece, 15, 5),
    new PieceWeight(ePieceClass_CastleSmallCorridorRightTurnPiece, 5, 10),
    new PieceWeight(ePieceClass_CastleSmallCorridorLeftTurnPiece, 5, 10),
    new PieceWeight(ePieceClass_CastleCorridorStairsPiece, 10, 3, true),
    new PieceWeight(ePieceClass_CastleCorridorTBalconyPiece, 7, 2),
};

void NetherBridgePieces::loadStatic()
{
    StructureFeatureIO::setPieceId(eStructurePiece_BridgeCrossing, &BridgeCrossing::Create, L"NeBCr");
    StructureFeatureIO::setPieceId(eStructurePiece_BridgeEndFiller, &BridgeEndFiller::Create, L"NeBEF");
    StructureFeatureIO::setPieceId(eStructurePiece_BridgeStraight, &BridgeStraight::Create, L"NeBS");
    StructureFeatureIO::setPieceId(eStructurePiece_CastleCorridorStairsPiece, &CastleCorridorStairsPiece::Create,
                                   L"NeCCS");
    StructureFeatureIO::setPieceId(eStructurePiece_CastleCorridorTBalconyPiece, &CastleCorridorTBalconyPiece::Create,
                                   L"NeCTB");
    StructureFeatureIO::setPieceId(eStructurePiece_CastleEntrance, &CastleEntrance::Create, L"NeCE");
    StructureFeatureIO::setPieceId(eStructurePiece_CastleSmallCorridorCrossingPiece,
                                   &CastleSmallCorridorCrossingPiece::Create, L"NeSCSC");
    StructureFeatureIO::setPieceId(eStructurePiece_CastleSmallCorridorLeftTurnPiece,
                                   &CastleSmallCorridorLeftTurnPiece::Create, L"NeSCLT");
    StructureFeatureIO::setPieceId(eStructurePiece_CastleSmallCorridorPiece, &CastleSmallCorridorPiece::Create,
                                   L"NeSC");
    StructureFeatureIO::setPieceId(eStructurePiece_CastleSmallCorridorRightTurnPiece,
                                   &CastleSmallCorridorRightTurnPiece::Create, L"NeSCRT");
    StructureFeatureIO::setPieceId(eStructurePiece_CastleStalkRoom, &CastleStalkRoom::Create, L"NeCSR");
    StructureFeatureIO::setPieceId(eStructurePiece_MonsterThrone, &MonsterThrone::Create, L"NeMT");
    StructureFeatureIO::setPieceId(eStructurePiece_RoomCrossing, &RoomCrossing::Create, L"NeRC");
    StructureFeatureIO::setPieceId(eStructurePiece_StairsRoom, &StairsRoom::Create, L"NeSR");
    StructureFeatureIO::setPieceId(eStructurePiece_NetherBridgeStartPiece, &BridgeCrossing::Create, L"NeStart");
}

StructurePiece* NetherBridgePieces::findAndCreateBridgePieceFactory(PieceWeight* weight,
                                                                    std::vector<StructurePiece*>* pieces,
                                                                    Random& random, int x, int y, int z,
                                                                    const Direction* direction, int depth)
{
    switch (weight->mPieceClass)
    {
    case ePieceClass_BridgeStraight:
        return BridgeStraight::createPiece(pieces, random, x, y, z, direction, depth);
    case ePieceClass_BridgeCrossing:
        return BridgeCrossing::createPiece(pieces, random, x, y, z, direction, depth);
    case ePieceClass_RoomCrossing:
        return RoomCrossing::createPiece(pieces, random, x, y, z, direction, depth);
    case ePieceClass_StairsRoom:
        return StairsRoom::createPiece(pieces, random, x, y, z, direction, depth);
    case ePieceClass_MonsterThrone:
        return MonsterThrone::createPiece(pieces, random, x, y, z, direction, depth);
    case ePieceClass_CastleEntrance:
        return CastleEntrance::createPiece(pieces, random, x, y, z, direction, depth);
    case ePieceClass_CastleSmallCorridorPiece:
        return CastleSmallCorridorPiece::createPiece(pieces, random, x, y, z, direction, depth);
    case ePieceClass_CastleSmallCorridorRightTurnPiece:
        return CastleSmallCorridorRightTurnPiece::createPiece(pieces, random, x, y, z, direction, depth);
    case ePieceClass_CastleSmallCorridorLeftTurnPiece:
        return CastleSmallCorridorLeftTurnPiece::createPiece(pieces, random, x, y, z, direction, depth);
    case ePieceClass_CastleCorridorStairsPiece:
        return CastleCorridorStairsPiece::createPiece(pieces, random, x, y, z, direction, depth);
    case ePieceClass_CastleCorridorTBalconyPiece:
        return CastleCorridorTBalconyPiece::createPiece(pieces, random, x, y, z, direction, depth);
    case ePieceClass_CastleSmallCorridorCrossingPiece:
        return CastleSmallCorridorCrossingPiece::createPiece(pieces, random, x, y, z, direction, depth);
    case ePieceClass_CastleStalkRoom:
        return CastleStalkRoom::createPiece(pieces, random, x, y, z, direction, depth);
    default:
        return NULL;
    }
}

NetherBridgePieces::NetherBridgePiece::NetherBridgePiece(int depth) : StructurePiece(depth)
{
}

NetherBridgePieces::NetherBridgePiece::NetherBridgePiece() : StructurePiece()
{
}

bool NetherBridgePieces::NetherBridgePiece::isOkBox(BoundingBox* box, StartPiece* start)
{
    if (box == NULL)
        return false;
    LevelData* levelData = start->mLevel->getLevelData();
    int half = ((levelData->getXZSize() / levelData->getHellScale()) << 4) >> 1;
    int lower = 1 - half;
    int upper = half - 1;
    bool result = box->x1 < upper && lower < box->z0 && lower < box->x0 && box->y0 > 10;
    if (upper <= box->z1)
        result = false;
    return result;
}

void NetherBridgePieces::NetherBridgePiece::readAdditionalSaveData(DataInputStream*, StructureManager*)
{
}

void NetherBridgePieces::NetherBridgePiece::addAdditionalSaveData(DataOutputStream*)
{
}

int NetherBridgePieces::NetherBridgePiece::updatePieceWeight(std::list<PieceWeight*>* weights)
{
    bool canPlace = false;
    int total = 0;
    for (std::list<PieceWeight*>::iterator it = weights->begin(); it != weights->end(); ++it)
    {
        PieceWeight* weight = *it;
        if (weight->mMaxPlaceCount > 0 && weight->mPlaceCount < weight->mMaxPlaceCount)
            canPlace = true;
        total += weight->mWeight;
    }
    return canPlace ? total : -1;
}

StructurePiece* NetherBridgePieces::NetherBridgePiece::generatePiece(StartPiece* start,
                                                                     std::list<PieceWeight*>* weights,
                                                                     std::vector<StructurePiece*>* pieces,
                                                                     Random& random, int x, int y, int z,
                                                                     const Direction* direction, int depth)
{
    int total = updatePieceWeight(weights);
    bool canPlace = total > 0 && depth <= 30;
    int attempt = 0;
    while (attempt < 5 && canPlace)
    {
        ++attempt;
        int choice = random.nextInt(total);
        for (std::list<PieceWeight*>::iterator it = weights->begin(); it != weights->end(); ++it)
        {
            PieceWeight* weight = *it;
            choice -= weight->mWeight;
            if (choice < 0)
            {
                if (!weight->doPlace(depth) || (weight == start->mPreviousPiece && !weight->mAllowInRow))
                    break;
                StructurePiece* piece = findAndCreateBridgePieceFactory(weight, pieces, random, x, y, z, direction,
                                                                        depth);
                if (piece != NULL)
                {
                    ++weight->mPlaceCount;
                    start->mPreviousPiece = weight;
                    if (!weight->isValid())
                        weights->remove(weight);
                    return piece;
                }
            }
        }
    }
    return BridgeEndFiller::createPiece(pieces, random, x, y, z, direction, depth);
}

StructurePiece* NetherBridgePieces::NetherBridgePiece::generateAndAddPiece(StartPiece* start,
                                                                           std::vector<StructurePiece*>* pieces,
                                                                           Random& random, int x, int y, int z,
                                                                           const Direction* direction, int depth,
                                                                           bool isCastle)
{
    if (std::abs(x - start->getBoundingBox()->x0) <= 112 && std::abs(z - start->getBoundingBox()->z0) <= 112)
    {
        std::list<PieceWeight*>* weights = &start->mAvailableBridgePieces;
        if (isCastle)
            weights = &start->mAvailableCastlePieces;
        StructurePiece* piece = generatePiece(start, weights, pieces, random, x, y, z, direction, depth + 1);
        if (piece != NULL)
        {
            pieces->push_back(piece);
            start->mPendingChildren.push_back(piece);
        }
        return piece;
    }
    return BridgeEndFiller::createPiece(pieces, random, x, y, z, direction, depth);
}

StructurePiece* NetherBridgePieces::NetherBridgePiece::generateChildForward(StartPiece* start,
                                                                            std::vector<StructurePiece*>* pieces,
                                                                            Random& random, int offsetX, int offsetY,
                                                                            bool isCastle)
{
    const Direction* direction = getOrientation();
    if (direction == NULL)
        return NULL;
    unsigned int value = direction->get3DDataValue();
    if (value <= 1)
        return NULL;
    if (value == 2)
    {
        return generateAndAddPiece(start, pieces, random, mBoundingBox->x0 + offsetX, mBoundingBox->y0 + offsetY,
                                   mBoundingBox->z0 - 1, direction, getGenDepth(), isCastle);
    }
    if (value < 4)
    {
        return generateAndAddPiece(start, pieces, random, mBoundingBox->x0 + offsetX, mBoundingBox->y0 + offsetY,
                                   mBoundingBox->z1 + 1, direction, getGenDepth(), isCastle);
    }
    if (value == 4)
    {
        return generateAndAddPiece(start, pieces, random, mBoundingBox->x0 - 1, mBoundingBox->y0 + offsetY,
                                   mBoundingBox->z0 + offsetX, direction, getGenDepth(), isCastle);
    }
    if (value == 5)
    {
        return generateAndAddPiece(start, pieces, random, mBoundingBox->x1 + 1, mBoundingBox->y0 + offsetY,
                                   mBoundingBox->z0 + offsetX, direction, getGenDepth(), isCastle);
    }
    return NULL;
}

StructurePiece* NetherBridgePieces::NetherBridgePiece::generateChildLeft(StartPiece* start,
                                                                         std::vector<StructurePiece*>* pieces,
                                                                         Random& random, int offsetY, int offsetXZ,
                                                                         bool isCastle)
{
    const Direction* direction = getOrientation();
    if (direction == NULL)
        return NULL;
    unsigned int value = direction->get3DDataValue();
    if (value <= 1)
        return NULL;
    if (value < 4)
    {
        return generateAndAddPiece(start, pieces, random, mBoundingBox->x0 - 1, mBoundingBox->y0 + offsetY,
                                   mBoundingBox->z0 + offsetXZ, Direction::WEST, getGenDepth(), isCastle);
    }
    if (value < 6)
    {
        return generateAndAddPiece(start, pieces, random, mBoundingBox->x0 + offsetXZ, mBoundingBox->y0 + offsetY,
                                   mBoundingBox->z0 - 1, Direction::NORTH, getGenDepth(), isCastle);
    }
    return NULL;
}

StructurePiece* NetherBridgePieces::NetherBridgePiece::generateChildRight(StartPiece* start,
                                                                          std::vector<StructurePiece*>* pieces,
                                                                          Random& random, int offsetY, int offsetXZ,
                                                                          bool isCastle)
{
    const Direction* direction = getOrientation();
    if (direction == NULL)
        return NULL;
    unsigned int value = direction->get3DDataValue();
    if (value <= 1)
        return NULL;
    if (value < 4)
    {
        return generateAndAddPiece(start, pieces, random, mBoundingBox->x1 + 1, mBoundingBox->y0 + offsetY,
                                   mBoundingBox->z0 + offsetXZ, Direction::EAST, getGenDepth(), isCastle);
    }
    if (value < 6)
    {
        return generateAndAddPiece(start, pieces, random, mBoundingBox->x0 + offsetXZ, mBoundingBox->y0 + offsetY,
                                   mBoundingBox->z1 + 1, Direction::SOUTH, getGenDepth(), isCastle);
    }
    return NULL;
}

void NetherBridgePieces::NetherBridgePiece::generateLightPost(Level* level, Random&, BoundingBox* box, int x, int y,
                                                              int z, int xOffset, int zOffset)
{
    const BlockPos& pos = BlockPos(getWorldX(x, z), getWorldY(y), getWorldZ(x, z));
    if (!box->isInside(pos))
        return;

    BlockPos above1 = pos.above();
    BlockPos above2 = pos.above(2);
    BlockPos above3 = pos.above(3);
    if (level->isEmptyBlock(pos) && level->isEmptyBlock(above1) && level->isEmptyBlock(above2)
        && level->isEmptyBlock(above3))
    {
        level->setBlock(pos, Blocks::nether_brick_fence->defaultBlockState(), Block::UPDATE_CLIENTS, false);
        level->setBlock(above1, Blocks::nether_brick_fence->defaultBlockState(), Block::UPDATE_CLIENTS, false);
        level->setBlock(above2, Blocks::nether_brick_fence->defaultBlockState(), Block::UPDATE_CLIENTS, false);
        level->setBlock(above3, Blocks::nether_brick_fence->defaultBlockState(), Block::UPDATE_CLIENTS, false);
        placeBlock(level, Blocks::nether_brick_fence->defaultBlockState(), x + xOffset, y + 3, z + zOffset, box);
        placeBlock(level, Blocks::glowstone->defaultBlockState(), x + xOffset, y + 2, z + zOffset, box);
    }
}

NetherBridgePieces::PieceWeight::PieceWeight(EPieceClass pieceClass, int weight, int maxPlaceCount, bool allowInRow)
    : mPieceClass(pieceClass),
      mWeight(weight),
      mPlaceCount(0),
      mMaxPlaceCount(maxPlaceCount),
      mAllowInRow(allowInRow)
{
}

NetherBridgePieces::PieceWeight::PieceWeight(EPieceClass pieceClass, int weight, int maxPlaceCount)
    : mPieceClass(pieceClass),
      mWeight(weight),
      mPlaceCount(0),
      mMaxPlaceCount(maxPlaceCount),
      mAllowInRow(false)
{
}

bool NetherBridgePieces::PieceWeight::doPlace(int)
{
    return mMaxPlaceCount == 0 || mPlaceCount < mMaxPlaceCount;
}

bool NetherBridgePieces::PieceWeight::isValid()
{
    return mMaxPlaceCount == 0 || mPlaceCount < mMaxPlaceCount;
}

static NetherBridgePieces::StartPiece* startPieceOf(std::vector<StructurePiece*>* pieces)
{
    if (pieces == NULL)
        return NULL;
    return static_cast<NetherBridgePieces::StartPiece*>(pieces->front());
}

NetherBridgePieces::BridgeCrossing::BridgeCrossing() : NetherBridgePiece()
{
}

NetherBridgePieces::BridgeCrossing::BridgeCrossing(Random& random, int x, int z) : NetherBridgePiece(0)
{
    setOrientation(Direction::Plane::HORIZONTAL->getRandomDirection(random));
    if (getOrientation()->getAxis() == Direction::Axis::Z)
        mBoundingBox = new BoundingBox(x, 64, z, x + 19 - 1, 73, z + 19 - 1);
    else
        mBoundingBox = new BoundingBox(x, 64, z, x + 19 - 1, 73, z + 19 - 1);
}

NetherBridgePieces::BridgeCrossing::BridgeCrossing(int depth, Random&, BoundingBox* box, const Direction* direction)
    : NetherBridgePiece(depth)
{
    setOrientation(direction);
    mBoundingBox = box;
}

StructurePiece* NetherBridgePieces::BridgeCrossing::createPiece(std::vector<StructurePiece*>* pieces, Random& random,
                                                                int x, int y, int z, const Direction* direction,
                                                                int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -8, -3, 0, 19, 10, 19, direction);
    if (isOkBox(box, startPieceOf(pieces)) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new BridgeCrossing(depth, random, box, direction);
    delete box;
    return NULL;
}

void NetherBridgePieces::BridgeCrossing::addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces,
                                                     Random& random)
{
    StartPiece* startPiece = static_cast<StartPiece*>(start);
    generateChildForward(startPiece, pieces, random, 8, 3, false);
    generateChildLeft(startPiece, pieces, random, 3, 8, false);
    generateChildRight(startPiece, pieces, random, 3, 8, false);
}

bool NetherBridgePieces::BridgeCrossing::postProcess(Level* level, Random&, BoundingBox* box)
{
    generateBox(level, box, 7, 3, 0, 11, 4, 18, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 3, 7, 18, 4, 11, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 8, 5, 0, 10, 7, 18, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 0, 5, 8, 18, 7, 10, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 7, 5, 0, 7, 5, 7, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 7, 5, 11, 7, 5, 18, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 11, 5, 0, 11, 5, 7, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 11, 5, 11, 11, 5, 18, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 5, 7, 7, 5, 7, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 11, 5, 7, 18, 5, 7, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 5, 11, 7, 5, 11, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 11, 5, 11, 18, 5, 11, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 7, 2, 0, 11, 2, 5, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 7, 2, 13, 11, 2, 18, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 7, 0, 0, 11, 1, 3, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 7, 0, 15, 11, 1, 18, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    for (int x = 7; x <= 11; ++x)
    {
        for (int z = 0; z <= 2; ++z)
        {
            fillColumnDown(level, Blocks::nether_brick->defaultBlockState(), x, -1, z, box);
            fillColumnDown(level, Blocks::nether_brick->defaultBlockState(), x, -1, 18 - z, box);
        }
    }
    generateBox(level, box, 0, 2, 7, 5, 2, 11, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 13, 2, 7, 18, 2, 11, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 0, 7, 3, 1, 11, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 15, 0, 7, 18, 1, 11, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    for (int x = 0; x <= 2; ++x)
    {
        for (int z = 7; z <= 11; ++z)
        {
            fillColumnDown(level, Blocks::nether_brick->defaultBlockState(), x, -1, z, box);
            fillColumnDown(level, Blocks::nether_brick->defaultBlockState(), 18 - x, -1, z, box);
        }
    }
    return true;
}

StructurePiece* NetherBridgePieces::BridgeCrossing::Create()
{
    return new BridgeCrossing();
}
EStructurePiece NetherBridgePieces::BridgeCrossing::GetType()
{
    return eStructurePiece_BridgeCrossing;
}

NetherBridgePieces::BridgeEndFiller::BridgeEndFiller() : NetherBridgePiece()
{
}

NetherBridgePieces::BridgeEndFiller::BridgeEndFiller(int depth, Random& random, BoundingBox* box,
                                                     const Direction* direction)
    : NetherBridgePiece(depth)
{
    setOrientation(direction);
    mBoundingBox = box;
    mSelfSeed = random.nextInt();
}

StructurePiece* NetherBridgePieces::BridgeEndFiller::createPiece(std::vector<StructurePiece*>* pieces, Random& random,
                                                                 int x, int y, int z, const Direction* direction,
                                                                 int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -1, -3, 0, 5, 10, 8, direction);
    if (isOkBox(box, startPieceOf(pieces)) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new BridgeEndFiller(depth, random, box, direction);
    delete box;
    return NULL;
}

void NetherBridgePieces::BridgeEndFiller::readAdditionalSaveData(DataInputStream* input, StructureManager*)
{
    mSelfSeed = input->readInt();
}

void NetherBridgePieces::BridgeEndFiller::addAdditionalSaveData(DataOutputStream* output)
{
    output->writeInt(mSelfSeed);
}

bool NetherBridgePieces::BridgeEndFiller::postProcess(Level* level, Random&, BoundingBox* box)
{
    Random random(static_cast<long long>(mSelfSeed));
    for (int x = 0; x <= 4; ++x)
    {
        for (int y = 3; y <= 4; ++y)
        {
            int length = random.nextInt(8);
            generateBox(level, box, x, y, 0, x, y, length, Blocks::nether_brick->defaultBlockState(),
                        Blocks::nether_brick->defaultBlockState(), false);
        }
    }

    int length = random.nextInt(8);
    generateBox(level, box, 0, 5, 0, 0, 5, length, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    length = random.nextInt(8);
    generateBox(level, box, 4, 5, 0, 4, 5, length, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);

    for (int x = 0; x <= 4; ++x)
    {
        int depth = random.nextInt(5);
        generateBox(level, box, x, 2, 0, x, 2, depth, Blocks::nether_brick->defaultBlockState(),
                    Blocks::nether_brick->defaultBlockState(), false);
    }

    for (int x = 0; x <= 4; ++x)
    {
        for (int y = 0; y <= 1; ++y)
        {
            int depth = random.nextInt(3);
            generateBox(level, box, x, y, 0, x, y, depth, Blocks::nether_brick->defaultBlockState(),
                        Blocks::nether_brick->defaultBlockState(), false);
        }
    }
    return true;
}

StructurePiece* NetherBridgePieces::BridgeEndFiller::Create()
{
    return new BridgeEndFiller();
}
EStructurePiece NetherBridgePieces::BridgeEndFiller::GetType()
{
    return eStructurePiece_BridgeEndFiller;
}

NetherBridgePieces::BridgeStraight::BridgeStraight() : NetherBridgePiece()
{
}

NetherBridgePieces::BridgeStraight::BridgeStraight(int depth, Random&, BoundingBox* box, const Direction* direction)
    : NetherBridgePiece(depth)
{
    setOrientation(direction);
    mBoundingBox = box;
}

StructurePiece* NetherBridgePieces::BridgeStraight::createPiece(std::vector<StructurePiece*>* pieces, Random& random,
                                                                int x, int y, int z, const Direction* direction,
                                                                int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -1, -3, 0, 5, 10, 19, direction);
    if (isOkBox(box, startPieceOf(pieces)) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new BridgeStraight(depth, random, box, direction);
    delete box;
    return NULL;
}

void NetherBridgePieces::BridgeStraight::addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces,
                                                     Random& random)
{
    generateChildForward(static_cast<StartPiece*>(start), pieces, random, 1, 3, false);
}

bool NetherBridgePieces::BridgeStraight::postProcess(Level* level, Random&, BoundingBox* box)
{
    generateBox(level, box, 0, 3, 0, 4, 4, 18, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 1, 5, 0, 3, 7, 18, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 0, 5, 0, 0, 5, 18, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 4, 5, 0, 4, 5, 18, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 2, 0, 4, 2, 5, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 2, 13, 4, 2, 18, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 0, 0, 4, 1, 3, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 0, 15, 4, 1, 18, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    for (int x = 0; x <= 4; ++x)
    {
        for (int z = 0; z <= 2; ++z)
        {
            fillColumnDown(level, Blocks::nether_brick->defaultBlockState(), x, -1, z, box);
            fillColumnDown(level, Blocks::nether_brick->defaultBlockState(), x, -1, 18 - z, box);
        }
    }
    generateBox(level, box, 0, 1, 1, 0, 4, 1, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    generateBox(level, box, 0, 3, 4, 0, 4, 4, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    generateBox(level, box, 0, 3, 14, 0, 4, 14, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    generateBox(level, box, 0, 1, 17, 0, 4, 17, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    generateBox(level, box, 4, 1, 1, 4, 4, 1, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    generateBox(level, box, 4, 3, 4, 4, 4, 4, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    generateBox(level, box, 4, 3, 14, 4, 4, 14, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    generateBox(level, box, 4, 1, 17, 4, 4, 17, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    return true;
}

StructurePiece* NetherBridgePieces::BridgeStraight::Create()
{
    return new BridgeStraight();
}
EStructurePiece NetherBridgePieces::BridgeStraight::GetType()
{
    return eStructurePiece_BridgeStraight;
}

NetherBridgePieces::CastleCorridorStairsPiece::CastleCorridorStairsPiece() : NetherBridgePiece()
{
}

NetherBridgePieces::CastleCorridorStairsPiece::CastleCorridorStairsPiece(int depth, Random&, BoundingBox* box,
                                                                         const Direction* direction)
    : NetherBridgePiece(depth)
{
    setOrientation(direction);
    mBoundingBox = box;
}

StructurePiece* NetherBridgePieces::CastleCorridorStairsPiece::createPiece(std::vector<StructurePiece*>* pieces,
                                                                           Random& random, int x, int y, int z,
                                                                           const Direction* direction, int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -1, -7, 0, 5, 14, 10, direction);
    if (isOkBox(box, startPieceOf(pieces)) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new CastleCorridorStairsPiece(depth, random, box, direction);
    delete box;
    return NULL;
}

void NetherBridgePieces::CastleCorridorStairsPiece::addChildren(StructurePiece* start,
                                                                std::vector<StructurePiece*>* pieces, Random& random)
{
    generateChildForward(static_cast<StartPiece*>(start), pieces, random, 1, 0, true);
}

bool NetherBridgePieces::CastleCorridorStairsPiece::postProcess(Level* level, Random&, BoundingBox* box)
{
    const BlockState* stairs = Blocks::nether_brick_stairs->defaultBlockState()->setValue(StairBlock::FACING,
                                                                                          Direction::SOUTH);
    for (int z = 0; z <= 9; ++z)
    {
        int floor = 7 - z < 1 ? 1 : 7 - z;
        int ceiling = std::min(std::max(floor + 5, 14 - z), 13);
        generateBox(level, box, 0, 0, z, 4, floor, z, Blocks::nether_brick->defaultBlockState(),
                    Blocks::nether_brick->defaultBlockState(), false);
        generateBox(level, box, 1, floor + 1, z, 3, ceiling - 1, z, Blocks::air->defaultBlockState(),
                    Blocks::air->defaultBlockState(), false);
        if (z <= 6)
        {
            placeBlock(level, stairs, 1, floor + 1, z, box);
            placeBlock(level, stairs, 2, floor + 1, z, box);
            placeBlock(level, stairs, 3, floor + 1, z, box);
        }
        generateBox(level, box, 0, ceiling, z, 4, ceiling, z, Blocks::nether_brick->defaultBlockState(),
                    Blocks::nether_brick->defaultBlockState(), false);
        generateBox(level, box, 0, floor + 1, z, 0, ceiling - 1, z, Blocks::nether_brick->defaultBlockState(),
                    Blocks::nether_brick->defaultBlockState(), false);
        generateBox(level, box, 4, floor + 1, z, 4, ceiling - 1, z, Blocks::nether_brick->defaultBlockState(),
                    Blocks::nether_brick->defaultBlockState(), false);
        if ((z & 1) == 0)
        {
            generateBox(level, box, 0, floor + 2, z, 0, floor + 3, z, Blocks::nether_brick_fence->defaultBlockState(),
                        Blocks::nether_brick_fence->defaultBlockState(), false);
            generateBox(level, box, 4, floor + 2, z, 4, floor + 3, z, Blocks::nether_brick_fence->defaultBlockState(),
                        Blocks::nether_brick_fence->defaultBlockState(), false);
        }
        for (int x = 0; x <= 4; ++x)
            fillColumnDown(level, Blocks::nether_brick->defaultBlockState(), x, -1, z, box);
    }
    return true;
}

StructurePiece* NetherBridgePieces::CastleCorridorStairsPiece::Create()
{
    return new CastleCorridorStairsPiece();
}
EStructurePiece NetherBridgePieces::CastleCorridorStairsPiece::GetType()
{
    return eStructurePiece_CastleCorridorStairsPiece;
}

NetherBridgePieces::CastleCorridorTBalconyPiece::CastleCorridorTBalconyPiece() : NetherBridgePiece()
{
}

NetherBridgePieces::CastleCorridorTBalconyPiece::CastleCorridorTBalconyPiece(int depth, Random&, BoundingBox* box,
                                                                             const Direction* direction)
    : NetherBridgePiece(depth)
{
    setOrientation(direction);
    mBoundingBox = box;
}

StructurePiece* NetherBridgePieces::CastleCorridorTBalconyPiece::createPiece(std::vector<StructurePiece*>* pieces,
                                                                             Random& random, int x, int y, int z,
                                                                             const Direction* direction, int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -3, 0, 0, 9, 7, 9, direction);
    if (isOkBox(box, startPieceOf(pieces)) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new CastleCorridorTBalconyPiece(depth, random, box, direction);
    delete box;
    return NULL;
}

void NetherBridgePieces::CastleCorridorTBalconyPiece::addChildren(StructurePiece* start,
                                                                  std::vector<StructurePiece*>* pieces, Random& random)
{
    int offset = 1;
    const Direction* orientation = getOrientation();
    if (orientation == Direction::WEST || orientation == Direction::NORTH)
        offset = 5;
    StartPiece* startPiece = static_cast<StartPiece*>(start);
    generateChildLeft(startPiece, pieces, random, 0, offset, random.nextInt(8) > 0);
    generateChildRight(startPiece, pieces, random, 0, offset, random.nextInt(8) > 0);
}

bool NetherBridgePieces::CastleCorridorTBalconyPiece::postProcess(Level* level, Random&, BoundingBox* box)
{
    generateBox(level, box, 0, 0, 0, 8, 1, 8, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 2, 0, 8, 5, 8, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 0, 6, 0, 8, 6, 5, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 2, 0, 2, 5, 0, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 6, 2, 0, 8, 5, 0, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 1, 3, 0, 1, 4, 0, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    generateBox(level, box, 7, 3, 0, 7, 4, 0, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    generateBox(level, box, 0, 2, 4, 8, 2, 8, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 1, 1, 4, 2, 2, 4, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 6, 1, 4, 7, 2, 4, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 0, 3, 8, 8, 3, 8, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    generateBox(level, box, 0, 3, 6, 0, 3, 7, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    generateBox(level, box, 8, 3, 6, 8, 3, 7, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    generateBox(level, box, 0, 3, 4, 0, 5, 5, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 8, 3, 4, 8, 5, 5, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 1, 3, 5, 2, 5, 5, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 6, 3, 5, 7, 5, 5, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 1, 4, 5, 1, 5, 5, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    generateBox(level, box, 7, 4, 5, 7, 5, 5, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    for (int z = 0; z <= 5; ++z)
    {
        for (int x = 0; x <= 8; ++x)
            fillColumnDown(level, Blocks::nether_brick->defaultBlockState(), x, -1, z, box);
    }
    return true;
}

StructurePiece* NetherBridgePieces::CastleCorridorTBalconyPiece::Create()
{
    return new CastleCorridorTBalconyPiece();
}
EStructurePiece NetherBridgePieces::CastleCorridorTBalconyPiece::GetType()
{
    return eStructurePiece_CastleCorridorTBalconyPiece;
}

NetherBridgePieces::CastleEntrance::CastleEntrance() : NetherBridgePiece()
{
}

NetherBridgePieces::CastleEntrance::CastleEntrance(int depth, Random&, BoundingBox* box, const Direction* direction)
    : NetherBridgePiece(depth)
{
    setOrientation(direction);
    mBoundingBox = box;
}

StructurePiece* NetherBridgePieces::CastleEntrance::createPiece(std::vector<StructurePiece*>* pieces, Random& random,
                                                                int x, int y, int z, const Direction* direction,
                                                                int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -5, -3, 0, 13, 14, 13, direction);
    if (isOkBox(box, startPieceOf(pieces)) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new CastleEntrance(depth, random, box, direction);
    delete box;
    return NULL;
}

void NetherBridgePieces::CastleEntrance::addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces,
                                                     Random& random)
{
    generateChildForward(static_cast<StartPiece*>(start), pieces, random, 5, 3, true);
}

bool NetherBridgePieces::CastleEntrance::postProcess(Level* level, Random& random, BoundingBox* box)
{
    generateBox(level, box, 0, 3, 0, 12, 4, 12, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 5, 0, 12, 13, 12, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 0, 5, 0, 1, 12, 12, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 11, 5, 0, 12, 12, 12, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 2, 5, 11, 4, 12, 12, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 8, 5, 11, 10, 12, 12, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 5, 9, 11, 7, 12, 12, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 2, 5, 0, 4, 12, 1, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 8, 5, 0, 10, 12, 1, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 5, 9, 0, 7, 12, 1, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 2, 11, 2, 10, 12, 10, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 5, 8, 0, 7, 8, 0, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    for (int i = 1; i <= 11; i += 2)
    {
        generateBox(level, box, i, 10, 0, i, 11, 0, Blocks::nether_brick_fence->defaultBlockState(),
                    Blocks::nether_brick_fence->defaultBlockState(), false);
        generateBox(level, box, i, 10, 12, i, 11, 12, Blocks::nether_brick_fence->defaultBlockState(),
                    Blocks::nether_brick_fence->defaultBlockState(), false);
        generateBox(level, box, 0, 10, i, 0, 11, i, Blocks::nether_brick_fence->defaultBlockState(),
                    Blocks::nether_brick_fence->defaultBlockState(), false);
        generateBox(level, box, 12, 10, i, 12, 11, i, Blocks::nether_brick_fence->defaultBlockState(),
                    Blocks::nether_brick_fence->defaultBlockState(), false);
        placeBlock(level, Blocks::nether_brick->defaultBlockState(), i, 13, 0, box);
        placeBlock(level, Blocks::nether_brick->defaultBlockState(), i, 13, 12, box);
        placeBlock(level, Blocks::nether_brick->defaultBlockState(), 0, 13, i, box);
        placeBlock(level, Blocks::nether_brick->defaultBlockState(), 12, 13, i, box);
        placeBlock(level, Blocks::nether_brick_fence->defaultBlockState(), i + 1, 13, 0, box);
        placeBlock(level, Blocks::nether_brick_fence->defaultBlockState(), i + 1, 13, 12, box);
        placeBlock(level, Blocks::nether_brick_fence->defaultBlockState(), 0, 13, i + 1, box);
        placeBlock(level, Blocks::nether_brick_fence->defaultBlockState(), 12, 13, i + 1, box);
    }
    placeBlock(level, Blocks::nether_brick_fence->defaultBlockState(), 0, 13, 0, box);
    placeBlock(level, Blocks::nether_brick_fence->defaultBlockState(), 0, 13, 12, box);
    placeBlock(level, Blocks::nether_brick_fence->defaultBlockState(), 0, 13, 0, box);
    placeBlock(level, Blocks::nether_brick_fence->defaultBlockState(), 12, 13, 0, box);
    for (int z = 3; z <= 9; z += 2)
    {
        generateBox(level, box, 1, 7, z, 1, 8, z, Blocks::nether_brick_fence->defaultBlockState(),
                    Blocks::nether_brick_fence->defaultBlockState(), false);
        generateBox(level, box, 11, 7, z, 11, 8, z, Blocks::nether_brick_fence->defaultBlockState(),
                    Blocks::nether_brick_fence->defaultBlockState(), false);
    }
    generateBox(level, box, 4, 2, 0, 8, 2, 12, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 2, 4, 12, 2, 8, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 4, 0, 0, 8, 1, 3, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 4, 0, 9, 8, 1, 12, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 0, 4, 3, 1, 8, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 9, 0, 4, 12, 1, 8, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    for (int x = 4; x <= 8; ++x)
    {
        for (int z = 0; z <= 2; ++z)
        {
            fillColumnDown(level, Blocks::nether_brick->defaultBlockState(), x, -1, z, box);
            fillColumnDown(level, Blocks::nether_brick->defaultBlockState(), x, -1, 12 - z, box);
        }
    }
    for (int x = 0; x <= 2; ++x)
    {
        for (int z = 4; z <= 8; ++z)
        {
            fillColumnDown(level, Blocks::nether_brick->defaultBlockState(), x, -1, z, box);
            fillColumnDown(level, Blocks::nether_brick->defaultBlockState(), 12 - x, -1, z, box);
        }
    }
    generateBox(level, box, 5, 5, 5, 7, 5, 7, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 6, 1, 6, 6, 4, 6, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    placeBlock(level, Blocks::nether_brick->defaultBlockState(), 6, 0, 6, box);
    const BlockState* lava = Blocks::flowing_lava->defaultBlockState();
    placeBlock(level, lava, 6, 5, 6, box);
    const BlockPos& pos = BlockPos(getWorldX(6, 6), getWorldY(5), getWorldZ(6, 6));
    if (box->isInside(pos))
        level->instaTick(pos, lava, random);
    return true;
}

StructurePiece* NetherBridgePieces::CastleEntrance::Create()
{
    return new CastleEntrance();
}
EStructurePiece NetherBridgePieces::CastleEntrance::GetType()
{
    return eStructurePiece_CastleEntrance;
}

NetherBridgePieces::CastleSmallCorridorCrossingPiece::CastleSmallCorridorCrossingPiece() : NetherBridgePiece()
{
}

NetherBridgePieces::CastleSmallCorridorCrossingPiece::CastleSmallCorridorCrossingPiece(int depth, Random&,
                                                                                       BoundingBox* box,
                                                                                       const Direction* direction)
    : NetherBridgePiece(depth)
{
    setOrientation(direction);
    mBoundingBox = box;
}

StructurePiece* NetherBridgePieces::CastleSmallCorridorCrossingPiece::createPiece(std::vector<StructurePiece*>* pieces,
                                                                                  Random& random, int x, int y, int z,
                                                                                  const Direction* direction, int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -1, 0, 0, 5, 7, 5, direction);
    if (isOkBox(box, startPieceOf(pieces)) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new CastleSmallCorridorCrossingPiece(depth, random, box, direction);
    delete box;
    return NULL;
}

void NetherBridgePieces::CastleSmallCorridorCrossingPiece::addChildren(StructurePiece* start,
                                                                       std::vector<StructurePiece*>* pieces,
                                                                       Random& random)
{
    StartPiece* startPiece = static_cast<StartPiece*>(start);
    generateChildForward(startPiece, pieces, random, 1, 0, true);
    generateChildLeft(startPiece, pieces, random, 0, 1, true);
    generateChildRight(startPiece, pieces, random, 0, 1, true);
}

bool NetherBridgePieces::CastleSmallCorridorCrossingPiece::postProcess(Level* level, Random&, BoundingBox* box)
{
    generateBox(level, box, 0, 0, 0, 4, 1, 4, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 2, 0, 4, 5, 4, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 0, 2, 0, 0, 5, 0, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 4, 2, 0, 4, 5, 0, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 2, 4, 0, 5, 4, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 4, 2, 4, 4, 5, 4, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 6, 0, 4, 6, 4, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    for (int x = 0; x <= 4; ++x)
    {
        for (int z = 0; z <= 4; ++z)
            fillColumnDown(level, Blocks::nether_brick->defaultBlockState(), x, -1, z, box);
    }
    return true;
}

StructurePiece* NetherBridgePieces::CastleSmallCorridorCrossingPiece::Create()
{
    return new CastleSmallCorridorCrossingPiece();
}
EStructurePiece NetherBridgePieces::CastleSmallCorridorCrossingPiece::GetType()
{
    return eStructurePiece_CastleSmallCorridorCrossingPiece;
}

NetherBridgePieces::CastleSmallCorridorPiece::CastleSmallCorridorPiece() : NetherBridgePiece()
{
}

NetherBridgePieces::CastleSmallCorridorPiece::CastleSmallCorridorPiece(int depth, Random&, BoundingBox* box,
                                                                       const Direction* direction)
    : NetherBridgePiece(depth)
{
    setOrientation(direction);
    mBoundingBox = box;
}

StructurePiece* NetherBridgePieces::CastleSmallCorridorPiece::createPiece(std::vector<StructurePiece*>* pieces,
                                                                          Random& random, int x, int y, int z,
                                                                          const Direction* direction, int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -1, 0, 0, 5, 7, 5, direction);
    if (isOkBox(box, startPieceOf(pieces)) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new CastleSmallCorridorPiece(depth, random, box, direction);
    delete box;
    return NULL;
}

void NetherBridgePieces::CastleSmallCorridorPiece::addChildren(StructurePiece* start,
                                                               std::vector<StructurePiece*>* pieces, Random& random)
{
    generateChildForward(static_cast<StartPiece*>(start), pieces, random, 1, 0, true);
}

bool NetherBridgePieces::CastleSmallCorridorPiece::postProcess(Level* level, Random&, BoundingBox* box)
{
    generateBox(level, box, 0, 0, 0, 4, 1, 4, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 2, 0, 4, 5, 4, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 0, 2, 0, 0, 5, 4, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 4, 2, 0, 4, 5, 4, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 3, 1, 0, 4, 1, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    generateBox(level, box, 0, 3, 3, 0, 4, 3, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    generateBox(level, box, 4, 3, 1, 4, 4, 1, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    generateBox(level, box, 4, 3, 3, 4, 4, 3, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    generateBox(level, box, 0, 6, 0, 4, 6, 4, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    for (int x = 0; x <= 4; ++x)
    {
        for (int z = 0; z <= 4; ++z)
            fillColumnDown(level, Blocks::nether_brick->defaultBlockState(), x, -1, z, box);
    }
    return true;
}

StructurePiece* NetherBridgePieces::CastleSmallCorridorPiece::Create()
{
    return new CastleSmallCorridorPiece();
}
EStructurePiece NetherBridgePieces::CastleSmallCorridorPiece::GetType()
{
    return eStructurePiece_CastleSmallCorridorPiece;
}

NetherBridgePieces::CastleStalkRoom::CastleStalkRoom() : NetherBridgePiece()
{
}

NetherBridgePieces::CastleStalkRoom::CastleStalkRoom(int depth, Random&, BoundingBox* box, const Direction* direction)
    : NetherBridgePiece(depth)
{
    setOrientation(direction);
    mBoundingBox = box;
}

StructurePiece* NetherBridgePieces::CastleStalkRoom::createPiece(std::vector<StructurePiece*>* pieces, Random& random,
                                                                 int x, int y, int z, const Direction* direction,
                                                                 int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -5, -3, 0, 13, 14, 13, direction);
    if (isOkBox(box, startPieceOf(pieces)) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new CastleStalkRoom(depth, random, box, direction);
    delete box;
    return NULL;
}

void NetherBridgePieces::CastleStalkRoom::addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces,
                                                      Random& random)
{
    StartPiece* startPiece = static_cast<StartPiece*>(start);
    generateChildForward(startPiece, pieces, random, 5, 3, true);
    generateChildForward(startPiece, pieces, random, 5, 11, true);
}

bool NetherBridgePieces::CastleStalkRoom::postProcess(Level* level, Random&, BoundingBox* box)
{
    generateBox(level, box, 0, 3, 0, 12, 4, 12, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 5, 0, 12, 13, 12, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 0, 5, 0, 1, 12, 12, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 11, 5, 0, 12, 12, 12, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 2, 5, 11, 4, 12, 12, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 8, 5, 11, 10, 12, 12, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 5, 9, 11, 7, 12, 12, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 2, 5, 0, 4, 12, 1, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 8, 5, 0, 10, 12, 1, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 5, 9, 0, 7, 12, 1, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 2, 11, 2, 10, 12, 10, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    for (int i = 1; i <= 11; i += 2)
    {
        generateBox(level, box, i, 10, 0, i, 11, 0, Blocks::nether_brick_fence->defaultBlockState(),
                    Blocks::nether_brick_fence->defaultBlockState(), false);
        generateBox(level, box, i, 10, 12, i, 11, 12, Blocks::nether_brick_fence->defaultBlockState(),
                    Blocks::nether_brick_fence->defaultBlockState(), false);
        generateBox(level, box, 0, 10, i, 0, 11, i, Blocks::nether_brick_fence->defaultBlockState(),
                    Blocks::nether_brick_fence->defaultBlockState(), false);
        generateBox(level, box, 12, 10, i, 12, 11, i, Blocks::nether_brick_fence->defaultBlockState(),
                    Blocks::nether_brick_fence->defaultBlockState(), false);
        placeBlock(level, Blocks::nether_brick->defaultBlockState(), i, 13, 0, box);
        placeBlock(level, Blocks::nether_brick->defaultBlockState(), i, 13, 12, box);
        placeBlock(level, Blocks::nether_brick->defaultBlockState(), 0, 13, i, box);
        placeBlock(level, Blocks::nether_brick->defaultBlockState(), 12, 13, i, box);
        placeBlock(level, Blocks::nether_brick_fence->defaultBlockState(), i + 1, 13, 0, box);
        placeBlock(level, Blocks::nether_brick_fence->defaultBlockState(), i + 1, 13, 12, box);
        placeBlock(level, Blocks::nether_brick_fence->defaultBlockState(), 0, 13, i + 1, box);
        placeBlock(level, Blocks::nether_brick_fence->defaultBlockState(), 12, 13, i + 1, box);
    }
    placeBlock(level, Blocks::nether_brick_fence->defaultBlockState(), 0, 13, 0, box);
    placeBlock(level, Blocks::nether_brick_fence->defaultBlockState(), 0, 13, 12, box);
    placeBlock(level, Blocks::nether_brick_fence->defaultBlockState(), 0, 13, 0, box);
    placeBlock(level, Blocks::nether_brick_fence->defaultBlockState(), 12, 13, 0, box);
    for (int z = 3; z <= 9; z += 2)
    {
        generateBox(level, box, 1, 7, z, 1, 8, z, Blocks::nether_brick_fence->defaultBlockState(),
                    Blocks::nether_brick_fence->defaultBlockState(), false);
        generateBox(level, box, 11, 7, z, 11, 8, z, Blocks::nether_brick_fence->defaultBlockState(),
                    Blocks::nether_brick_fence->defaultBlockState(), false);
    }

    const BlockState* stairsNorth = Blocks::nether_brick_stairs->defaultBlockState()->setValue(StairBlock::FACING,
                                                                                               Direction::NORTH);
    for (int step = 0; step <= 6; ++step)
    {
        int z = step + 4;
        for (int x = 5; x <= 7; ++x)
            placeBlock(level, stairsNorth, x, 5 + step, z, box);
        if (z >= 5 && z <= 8)
            generateBox(level, box, 5, 5, z, 7, step + 4, z, Blocks::nether_brick->defaultBlockState(),
                        Blocks::nether_brick->defaultBlockState(), false);
        else if (z >= 9 && z <= 10)
            generateBox(level, box, 5, 8, z, 7, step + 4, z, Blocks::nether_brick->defaultBlockState(),
                        Blocks::nether_brick->defaultBlockState(), false);
        if (step >= 1)
            generateBox(level, box, 5, 6 + step, z, 7, 9 + step, z, Blocks::air->defaultBlockState(),
                        Blocks::air->defaultBlockState(), false);
    }
    for (int x = 5; x <= 7; ++x)
        placeBlock(level, stairsNorth, x, 12, 11, box);
    generateBox(level, box, 5, 6, 7, 5, 7, 7, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    generateBox(level, box, 7, 6, 7, 7, 7, 7, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    generateBox(level, box, 5, 13, 12, 7, 13, 12, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 2, 5, 2, 3, 5, 3, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 2, 5, 9, 3, 5, 10, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 2, 5, 4, 2, 5, 8, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 9, 5, 2, 10, 5, 3, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 9, 5, 9, 10, 5, 10, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 10, 5, 4, 10, 5, 8, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);

    const BlockState* stairsEast = stairsNorth->setValue(StairBlock::FACING, Direction::EAST);
    const BlockState* stairsWest = stairsNorth->setValue(StairBlock::FACING, Direction::WEST);
    placeBlock(level, stairsWest, 4, 5, 2, box);
    placeBlock(level, stairsWest, 4, 5, 3, box);
    placeBlock(level, stairsWest, 4, 5, 9, box);
    placeBlock(level, stairsWest, 4, 5, 10, box);
    placeBlock(level, stairsEast, 8, 5, 2, box);
    placeBlock(level, stairsEast, 8, 5, 3, box);
    placeBlock(level, stairsEast, 8, 5, 9, box);
    placeBlock(level, stairsEast, 8, 5, 10, box);
    generateBox(level, box, 3, 4, 4, 4, 4, 8, Blocks::soul_sand->defaultBlockState(),
                Blocks::soul_sand->defaultBlockState(), false);
    generateBox(level, box, 8, 4, 4, 9, 4, 8, Blocks::soul_sand->defaultBlockState(),
                Blocks::soul_sand->defaultBlockState(), false);
    generateBox(level, box, 3, 5, 4, 4, 5, 8, Blocks::nether_wart->defaultBlockState(),
                Blocks::nether_wart->defaultBlockState(), false);
    generateBox(level, box, 8, 5, 4, 9, 5, 8, Blocks::nether_wart->defaultBlockState(),
                Blocks::nether_wart->defaultBlockState(), false);
    generateBox(level, box, 4, 2, 0, 8, 2, 12, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 2, 4, 12, 2, 8, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 4, 0, 0, 8, 1, 3, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 4, 0, 9, 8, 1, 12, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 0, 4, 3, 1, 8, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 9, 0, 4, 12, 1, 8, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    for (int x = 4; x <= 8; ++x)
    {
        for (int z = 0; z <= 2; ++z)
        {
            fillColumnDown(level, Blocks::nether_brick->defaultBlockState(), x, -1, z, box);
            fillColumnDown(level, Blocks::nether_brick->defaultBlockState(), x, -1, 12 - z, box);
        }
    }
    for (int x = 0; x <= 2; ++x)
    {
        for (int z = 4; z <= 8; ++z)
        {
            fillColumnDown(level, Blocks::nether_brick->defaultBlockState(), x, -1, z, box);
            fillColumnDown(level, Blocks::nether_brick->defaultBlockState(), 12 - x, -1, z, box);
        }
    }
    return true;
}

StructurePiece* NetherBridgePieces::CastleStalkRoom::Create()
{
    return new CastleStalkRoom();
}
EStructurePiece NetherBridgePieces::CastleStalkRoom::GetType()
{
    return eStructurePiece_CastleStalkRoom;
}

NetherBridgePieces::RoomCrossing::RoomCrossing() : NetherBridgePiece()
{
}

NetherBridgePieces::RoomCrossing::RoomCrossing(int depth, Random&, BoundingBox* box, const Direction* direction)
    : NetherBridgePiece(depth)
{
    setOrientation(direction);
    mBoundingBox = box;
}

StructurePiece* NetherBridgePieces::RoomCrossing::createPiece(std::vector<StructurePiece*>* pieces, Random& random,
                                                              int x, int y, int z, const Direction* direction,
                                                              int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -2, 0, 0, 7, 9, 7, direction);
    if (isOkBox(box, startPieceOf(pieces)) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new RoomCrossing(depth, random, box, direction);
    delete box;
    return NULL;
}

void NetherBridgePieces::RoomCrossing::addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces,
                                                   Random& random)
{
    StartPiece* startPiece = static_cast<StartPiece*>(start);
    generateChildForward(startPiece, pieces, random, 2, 0, false);
    generateChildLeft(startPiece, pieces, random, 0, 2, false);
    generateChildRight(startPiece, pieces, random, 0, 2, false);
}

bool NetherBridgePieces::RoomCrossing::postProcess(Level* level, Random&, BoundingBox* box)
{
    generateBox(level, box, 0, 0, 0, 6, 1, 6, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 2, 0, 6, 7, 6, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 0, 2, 0, 1, 6, 0, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 2, 6, 1, 6, 6, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 5, 2, 0, 6, 6, 0, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 5, 2, 6, 6, 6, 6, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 2, 0, 0, 6, 1, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 2, 5, 0, 6, 6, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 6, 2, 0, 6, 6, 1, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 6, 2, 5, 6, 6, 6, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 2, 6, 0, 4, 6, 0, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 2, 5, 0, 4, 5, 0, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    generateBox(level, box, 2, 6, 6, 4, 6, 6, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 2, 5, 6, 4, 5, 6, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    generateBox(level, box, 0, 6, 2, 0, 6, 4, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 5, 2, 0, 5, 4, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    generateBox(level, box, 6, 6, 2, 6, 6, 4, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 6, 5, 2, 6, 5, 4, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    for (int x = 0; x <= 6; ++x)
    {
        for (int z = 0; z <= 6; ++z)
            fillColumnDown(level, Blocks::nether_brick->defaultBlockState(), x, -1, z, box);
    }
    return true;
}

StructurePiece* NetherBridgePieces::RoomCrossing::Create()
{
    return new RoomCrossing();
}
EStructurePiece NetherBridgePieces::RoomCrossing::GetType()
{
    return eStructurePiece_RoomCrossing;
}

NetherBridgePieces::StairsRoom::StairsRoom() : NetherBridgePiece()
{
}

NetherBridgePieces::StairsRoom::StairsRoom(int depth, Random&, BoundingBox* box, const Direction* direction)
    : NetherBridgePiece(depth)
{
    setOrientation(direction);
    mBoundingBox = box;
}

StructurePiece* NetherBridgePieces::StairsRoom::createPiece(std::vector<StructurePiece*>* pieces, Random& random, int x,
                                                            int y, int z, const Direction* direction, int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -2, 0, 0, 7, 11, 7, direction);
    if (isOkBox(box, startPieceOf(pieces)) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new StairsRoom(depth, random, box, direction);
    delete box;
    return NULL;
}

void NetherBridgePieces::StairsRoom::addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces,
                                                 Random& random)
{
    generateChildRight(static_cast<StartPiece*>(start), pieces, random, 6, 2, false);
}

bool NetherBridgePieces::StairsRoom::postProcess(Level* level, Random&, BoundingBox* box)
{
    generateBox(level, box, 0, 0, 0, 6, 1, 6, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 2, 0, 6, 10, 6, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 0, 2, 0, 1, 8, 0, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 5, 2, 0, 6, 8, 0, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 2, 1, 0, 8, 6, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 6, 2, 1, 6, 8, 6, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 1, 2, 6, 5, 8, 6, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 3, 2, 0, 5, 4, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    generateBox(level, box, 6, 3, 2, 6, 5, 2, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    generateBox(level, box, 6, 3, 4, 6, 5, 4, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    placeBlock(level, Blocks::nether_brick->defaultBlockState(), 5, 2, 5, box);
    generateBox(level, box, 4, 2, 5, 4, 3, 5, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 3, 2, 5, 3, 4, 5, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 2, 2, 5, 2, 5, 5, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 1, 2, 5, 1, 6, 5, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 1, 7, 1, 5, 7, 4, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 6, 8, 2, 6, 8, 4, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 2, 6, 0, 4, 8, 0, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 2, 5, 0, 4, 5, 0, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    for (int x = 0; x <= 6; ++x)
    {
        for (int z = 0; z <= 6; ++z)
            fillColumnDown(level, Blocks::nether_brick->defaultBlockState(), x, -1, z, box);
    }
    return true;
}

StructurePiece* NetherBridgePieces::StairsRoom::Create()
{
    return new StairsRoom();
}
EStructurePiece NetherBridgePieces::StairsRoom::GetType()
{
    return eStructurePiece_StairsRoom;
}

NetherBridgePieces::CastleSmallCorridorLeftTurnPiece::CastleSmallCorridorLeftTurnPiece()
    : NetherBridgePiece(),
      mIsNeedingChest(false)
{
}

NetherBridgePieces::CastleSmallCorridorLeftTurnPiece::CastleSmallCorridorLeftTurnPiece(int depth, Random& random,
                                                                                       BoundingBox* box,
                                                                                       const Direction* direction)
    : NetherBridgePiece(depth)
{
    setOrientation(direction);
    mBoundingBox = box;
    mIsNeedingChest = random.nextInt(3) == 0;
}

StructurePiece* NetherBridgePieces::CastleSmallCorridorLeftTurnPiece::createPiece(std::vector<StructurePiece*>* pieces,
                                                                                  Random& random, int x, int y, int z,
                                                                                  const Direction* direction, int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -1, 0, 0, 5, 7, 5, direction);
    if (isOkBox(box, startPieceOf(pieces)) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new CastleSmallCorridorLeftTurnPiece(depth, random, box, direction);
    delete box;
    return NULL;
}

void NetherBridgePieces::CastleSmallCorridorLeftTurnPiece::readAdditionalSaveData(DataInputStream* input,
                                                                                  StructureManager*)
{
    mIsNeedingChest = input->readBoolean();
}

void NetherBridgePieces::CastleSmallCorridorLeftTurnPiece::addAdditionalSaveData(DataOutputStream* output)
{
    output->writeBoolean(mIsNeedingChest);
}

void NetherBridgePieces::CastleSmallCorridorLeftTurnPiece::addChildren(StructurePiece* start,
                                                                       std::vector<StructurePiece*>* pieces,
                                                                       Random& random)
{
    generateChildLeft(static_cast<StartPiece*>(start), pieces, random, 0, 1, true);
}

bool NetherBridgePieces::CastleSmallCorridorLeftTurnPiece::postProcess(Level* level, Random& random, BoundingBox* box)
{
    generateBox(level, box, 0, 0, 0, 4, 1, 4, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 2, 0, 4, 5, 4, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 4, 2, 0, 4, 5, 4, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 4, 3, 1, 4, 4, 1, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    generateBox(level, box, 4, 3, 3, 4, 4, 3, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    generateBox(level, box, 0, 2, 0, 0, 5, 0, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 2, 4, 3, 5, 4, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 1, 3, 4, 1, 4, 4, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 3, 3, 4, 3, 4, 4, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    if (mIsNeedingChest)
    {
        const BlockPos& pos = BlockPos(getWorldX(3, 3), getWorldY(2), getWorldZ(3, 3));
        if (box->isInside(pos))
        {
            setDirty();
            mIsNeedingChest = false;
            createChest(level, box, random, 3, 2, 3, &BuiltInLootTables::NETHER_BRIDGE);
        }
    }
    generateBox(level, box, 0, 6, 0, 4, 6, 4, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    for (int x = 0; x <= 4; ++x)
    {
        for (int z = 0; z <= 4; ++z)
            fillColumnDown(level, Blocks::nether_brick->defaultBlockState(), x, -1, z, box);
    }
    return true;
}

StructurePiece* NetherBridgePieces::CastleSmallCorridorLeftTurnPiece::Create()
{
    return new CastleSmallCorridorLeftTurnPiece();
}
EStructurePiece NetherBridgePieces::CastleSmallCorridorLeftTurnPiece::GetType()
{
    return eStructurePiece_CastleSmallCorridorLeftTurnPiece;
}

NetherBridgePieces::CastleSmallCorridorRightTurnPiece::CastleSmallCorridorRightTurnPiece()
    : NetherBridgePiece(),
      mIsNeedingChest(false)
{
}

NetherBridgePieces::CastleSmallCorridorRightTurnPiece::CastleSmallCorridorRightTurnPiece(int depth, Random& random,
                                                                                         BoundingBox* box,
                                                                                         const Direction* direction)
    : NetherBridgePiece(depth)
{
    setOrientation(direction);
    mBoundingBox = box;
    mIsNeedingChest = random.nextInt(3) == 0;
}

StructurePiece* NetherBridgePieces::CastleSmallCorridorRightTurnPiece::createPiece(std::vector<StructurePiece*>* pieces,
                                                                                   Random& random, int x, int y, int z,
                                                                                   const Direction* direction,
                                                                                   int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -1, 0, 0, 5, 7, 5, direction);
    if (isOkBox(box, startPieceOf(pieces)) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new CastleSmallCorridorRightTurnPiece(depth, random, box, direction);
    delete box;
    return NULL;
}

void NetherBridgePieces::CastleSmallCorridorRightTurnPiece::readAdditionalSaveData(DataInputStream* input,
                                                                                   StructureManager*)
{
    mIsNeedingChest = input->readBoolean();
}

void NetherBridgePieces::CastleSmallCorridorRightTurnPiece::addAdditionalSaveData(DataOutputStream* output)
{
    output->writeBoolean(mIsNeedingChest);
}

void NetherBridgePieces::CastleSmallCorridorRightTurnPiece::addChildren(StructurePiece* start,
                                                                        std::vector<StructurePiece*>* pieces,
                                                                        Random& random)
{
    generateChildRight(static_cast<StartPiece*>(start), pieces, random, 0, 1, true);
}

bool NetherBridgePieces::CastleSmallCorridorRightTurnPiece::postProcess(Level* level, Random& random, BoundingBox* box)
{
    generateBox(level, box, 0, 0, 0, 4, 1, 4, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 2, 0, 4, 5, 4, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 0, 2, 0, 0, 5, 4, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 3, 1, 0, 4, 1, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    generateBox(level, box, 0, 3, 3, 0, 4, 3, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    generateBox(level, box, 4, 2, 0, 4, 5, 0, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 1, 2, 4, 4, 5, 4, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 1, 3, 4, 1, 4, 4, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 3, 3, 4, 3, 4, 4, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    if (mIsNeedingChest)
    {
        const BlockPos& pos = BlockPos(getWorldX(1, 3), getWorldY(2), getWorldZ(1, 3));
        if (box->isInside(pos))
        {
            setDirty();
            mIsNeedingChest = false;
            createChest(level, box, random, 1, 2, 3, &BuiltInLootTables::NETHER_BRIDGE);
        }
    }
    generateBox(level, box, 0, 6, 0, 4, 6, 4, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    for (int x = 0; x <= 4; ++x)
    {
        for (int z = 0; z <= 4; ++z)
            fillColumnDown(level, Blocks::nether_brick->defaultBlockState(), x, -1, z, box);
    }
    return true;
}

StructurePiece* NetherBridgePieces::CastleSmallCorridorRightTurnPiece::Create()
{
    return new CastleSmallCorridorRightTurnPiece();
}
EStructurePiece NetherBridgePieces::CastleSmallCorridorRightTurnPiece::GetType()
{
    return eStructurePiece_CastleSmallCorridorRightTurnPiece;
}

NetherBridgePieces::MonsterThrone::MonsterThrone() : NetherBridgePiece()
{
}

NetherBridgePieces::MonsterThrone::MonsterThrone(int depth, Random&, BoundingBox* box, const Direction* direction)
    : NetherBridgePiece(depth)
{
    setOrientation(direction);
    mBoundingBox = box;
    mHasPlacedSpawner = false;
}

StructurePiece* NetherBridgePieces::MonsterThrone::createPiece(std::vector<StructurePiece*>* pieces, Random& random,
                                                               int x, int y, int z, const Direction* direction,
                                                               int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -2, 0, 0, 7, 8, 9, direction);
    if (isOkBox(box, startPieceOf(pieces)) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new MonsterThrone(depth, random, box, direction);
    delete box;
    return NULL;
}

void NetherBridgePieces::MonsterThrone::readAdditionalSaveData(DataInputStream* input, StructureManager*)
{
    mHasPlacedSpawner = input->readBoolean();
}

void NetherBridgePieces::MonsterThrone::addAdditionalSaveData(DataOutputStream* output)
{
    output->writeBoolean(mHasPlacedSpawner);
}

bool NetherBridgePieces::MonsterThrone::postProcess(Level* level, Random&, BoundingBox* box)
{
    generateBox(level, box, 0, 2, 0, 6, 7, 7, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 1, 0, 0, 5, 1, 7, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 1, 2, 1, 5, 2, 7, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 1, 3, 2, 5, 3, 7, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 1, 4, 3, 5, 4, 7, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 1, 2, 0, 1, 4, 2, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 5, 2, 0, 5, 4, 2, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 1, 5, 2, 1, 5, 3, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 5, 5, 2, 5, 5, 3, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 0, 5, 3, 0, 5, 8, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 6, 5, 3, 6, 5, 8, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    generateBox(level, box, 1, 5, 8, 5, 5, 8, Blocks::nether_brick->defaultBlockState(),
                Blocks::nether_brick->defaultBlockState(), false);
    placeBlock(level, Blocks::nether_brick_fence->defaultBlockState(), 1, 6, 3, box);
    placeBlock(level, Blocks::nether_brick_fence->defaultBlockState(), 5, 6, 3, box);
    generateBox(level, box, 0, 6, 3, 0, 6, 8, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    generateBox(level, box, 6, 6, 3, 6, 6, 8, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    generateBox(level, box, 1, 6, 8, 5, 7, 8, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    generateBox(level, box, 2, 8, 8, 4, 8, 8, Blocks::nether_brick_fence->defaultBlockState(),
                Blocks::nether_brick_fence->defaultBlockState(), false);
    if (!mHasPlacedSpawner)
    {
        const BlockPos& pos = BlockPos(getWorldX(3, 5), getWorldY(5), getWorldZ(3, 5));
        if (box->isInside(pos))
        {
            setDirty();
            mHasPlacedSpawner = true;
            level->setBlock(pos, Blocks::mob_spawner->defaultBlockState(), Block::UPDATE_CLIENTS, false);
            boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
            if (blockEntity != NULL && eTYPE_DERIVED_FROM(BlockEntityType::MOB_SPAWNER, blockEntity->GetType()))
            {
                boost::shared_ptr<MobSpawnerBlockEntity> spawner = boost::static_pointer_cast<MobSpawnerBlockEntity>(
                    blockEntity);
                spawner->getSpawner()->setEntityId(EntityIO::getKey(EntityType::BLAZE));
            }
        }
    }
    for (int x = 0; x <= 6; ++x)
    {
        for (int z = 0; z <= 6; ++z)
            fillColumnDown(level, Blocks::nether_brick->defaultBlockState(), x, -1, z, box);
    }
    return true;
}

StructurePiece* NetherBridgePieces::MonsterThrone::Create()
{
    return new MonsterThrone();
}
EStructurePiece NetherBridgePieces::MonsterThrone::GetType()
{
    return eStructurePiece_MonsterThrone;
}

NetherBridgePieces::StartPiece::StartPiece(Random& random, int x, int z, Level* level)
    : BridgeCrossing(random, x, z),
      mAvailableBridgePieces(),
      mAvailableCastlePieces(),
      mPendingChildren()
{
    mLevel = level;
    mPreviousPiece = NULL;
    for (int i = 0; i < 6; ++i)
    {
        PieceWeight* weight = bridgePieceWeights[i];
        weight->mPlaceCount = 0;
        mAvailableBridgePieces.push_back(weight);
    }
    for (int i = 0; i < 7; ++i)
    {
        PieceWeight* weight = castlePieceWeights[i];
        weight->mPlaceCount = 0;
        mAvailableCastlePieces.push_back(weight);
    }
}

NetherBridgePieces::StartPiece::StartPiece()
    : BridgeCrossing(),
      mPreviousPiece(NULL),
      mLevel(NULL),
      mAvailableBridgePieces(),
      mAvailableCastlePieces(),
      mPendingChildren()
{
}

EStructurePiece NetherBridgePieces::StartPiece::GetType()
{
    return eStructurePiece_NetherBridgeStartPiece;
}

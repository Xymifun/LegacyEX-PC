#include "game/core/StrongholdPieces.hpp"
#include "game/core/EntityType.hpp"

#include <cstdlib>
#include <list>

#include "game/core/BuiltInLootTables.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/generation/StructureFeatureIO.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/ButtonBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/DoorBlock.hpp"
#include "game/world/block/EndPortalFrameBlock.hpp"
#include "game/world/block/LadderBlock.hpp"
#include "game/world/block/MobSpawnerBlockEntity.hpp"
#include "game/world/block/MonsterEggBlock.hpp"
#include "game/world/block/StairBlock.hpp"
#include "game/world/block/StoneBrickBlock.hpp"
#include "game/world/block/StoneSlabBlock.hpp"
#include "game/world/block/TorchBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

static int sStrongholdWeightTotal = 0;
static StrongholdPieces::SmoothStoneSelector* sStrongholdStoneSelector = NULL;
static std::list<StrongholdPieces::PieceWeight*> sStrongholdPieceWeights;
static StrongholdPieces::EPieceClass sForcedPieceClass = StrongholdPieces::ePieceClass_None;

StrongholdPieces::PieceWeight::PieceWeight(EPieceClass pieceClass, int weight, int maxPlaceCount)
    : pieceClass(pieceClass),
      weight(weight),
      placeCount(0),
      maxPlaceCount(maxPlaceCount)
{
}

StrongholdPieces::PieceWeight::~PieceWeight()
{
}

bool StrongholdPieces::PieceWeight::isValid()
{
    return maxPlaceCount == 0 || placeCount < maxPlaceCount;
}

bool StrongholdPieces::PieceWeight::doPlace(int depth)
{
    return maxPlaceCount == 0 || placeCount < maxPlaceCount;
}

StrongholdPieces::PieceWeight_Library::~PieceWeight_Library()
{
}

bool StrongholdPieces::PieceWeight_Library::doPlace(int depth)
{
    return PieceWeight::doPlace(depth) && depth > 4;
}

StrongholdPieces::PieceWeight_PortalRoom::~PieceWeight_PortalRoom()
{
}

bool StrongholdPieces::PieceWeight_PortalRoom::doPlace(int depth)
{
    return PieceWeight::doPlace(depth) && depth > 5;
}

StrongholdPieces::StrongholdPiece::StrongholdPiece(int depth) : StructurePiece(depth), mEntryDoor(OPENING)
{
}

StrongholdPieces::StrongholdPiece::StrongholdPiece() : StructurePiece(), mEntryDoor(OPENING)
{
}

bool StrongholdPieces::StrongholdPiece::isOkBox(BoundingBox* box, StartPiece* start)
{
    bool result = false;
    if (box != NULL)
    {
        if (box->y0 > 10)
            result = true;
        if (start != NULL && start->mLevel->getOriginalSaveVersion() >= 5)
        {
            int halfSize = start->mLevel->getLevelData()->getXZSize() * 16 / 2;
            int minBound = 1 - halfSize;
            int maxBound = halfSize - 1;
            if (box->x0 <= minBound)
                result = false;
            if (box->z0 <= minBound)
                result = false;
            if (box->x1 >= maxBound)
                result = false;
            if (box->z1 >= maxBound)
                result = false;
        }
    }
    return result;
}

StrongholdPieces::StrongholdPiece::SmallDoorType StrongholdPieces::StrongholdPiece::randomSmallDoor(Random& random)
{
    switch (random.nextInt(5))
    {
    default:
        return OPENING;
    case 2:
        return WOOD_DOOR;
    case 3:
        return GRATES;
    case 4:
        return IRON_DOOR;
    }
}

void StrongholdPieces::StrongholdPiece::addAdditionalSaveData(DataOutputStream* output)
{
    output->writeInt(mEntryDoor);
}

void StrongholdPieces::StrongholdPiece::readAdditionalSaveData(DataInputStream* input, StructureManager*)
{
    mEntryDoor = static_cast<SmallDoorType>(input->readInt());
}

void StrongholdPieces::StrongholdPiece::generateSmallDoor(Level* level, Random&, BoundingBox* box, SmallDoorType type,
                                                          int x, int y, int z)
{
    if (type == WOOD_DOOR)
    {
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), x, y, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), x, y + 1, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), x, y + 2, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), x + 1, y + 2, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), x + 2, y + 2, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), x + 2, y + 1, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), x + 2, y, z, box);
        placeBlock(level, Blocks::wooden_door->defaultBlockState(), x + 1, y, z, box);
        TypedBoxed<DoorBlock::DoorHalf*> upperHalf(DoorBlock::UPPER);
        placeBlock(level, Blocks::wooden_door->defaultBlockState()->setBoxedValue(DoorBlock::HALF, &upperHalf), x + 1,
                   y + 1, z, box);
        return;
    }
    if (type == GRATES)
    {
        placeBlock(level, Blocks::air->defaultBlockState(), x + 1, y, z, box);
        placeBlock(level, Blocks::air->defaultBlockState(), x + 1, y + 1, z, box);
        placeBlock(level, Blocks::iron_bars->defaultBlockState(), x, y, z, box);
        placeBlock(level, Blocks::iron_bars->defaultBlockState(), x, y + 1, z, box);
        placeBlock(level, Blocks::iron_bars->defaultBlockState(), x, y + 2, z, box);
        placeBlock(level, Blocks::iron_bars->defaultBlockState(), x + 1, y + 2, z, box);
        placeBlock(level, Blocks::iron_bars->defaultBlockState(), x + 2, y + 2, z, box);
        placeBlock(level, Blocks::iron_bars->defaultBlockState(), x + 2, y + 1, z, box);
        placeBlock(level, Blocks::iron_bars->defaultBlockState(), x + 2, y, z, box);
        return;
    }
    if (type == IRON_DOOR)
    {
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), x, y, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), x, y + 1, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), x, y + 2, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), x + 1, y + 2, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), x + 2, y + 2, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), x + 2, y + 1, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), x + 2, y, z, box);
        placeBlock(level, Blocks::iron_door->defaultBlockState(), x + 1, y, z, box);
        TypedBoxed<DoorBlock::DoorHalf*> upperHalf(DoorBlock::UPPER);
        placeBlock(level, Blocks::iron_door->defaultBlockState()->setBoxedValue(DoorBlock::HALF, &upperHalf), x + 1,
                   y + 1, z, box);
        TypedBoxed<const Direction*> buttonNorth(Direction::NORTH);
        placeBlock(level, Blocks::stone_button->defaultBlockState()->setBoxedValue(ButtonBlock::FACING, &buttonNorth),
                   x + 2, y + 1, z + 1, box);
        TypedBoxed<const Direction*> buttonSouth(Direction::SOUTH);
        placeBlock(level, Blocks::stone_button->defaultBlockState()->setBoxedValue(ButtonBlock::FACING, &buttonSouth),
                   x + 2, y + 1, z - 1, box);
        return;
    }
    generateBox(level, box, x, y, z, x + 2, y + 2, z, Blocks::air->defaultBlockState(),
                Blocks::air->defaultBlockState(), false);
}

StructurePiece* StrongholdPieces::StrongholdPiece::generateSmallDoorChildForward(StartPiece* start,
                                                                                 std::vector<StructurePiece*>* pieces,
                                                                                 Random& random, int offsetX,
                                                                                 int offsetY)
{
    const Direction* direction = getOrientation();
    if (direction == NULL)
        return NULL;
    unsigned int value = direction->get3DDataValue();
    if (value <= 1)
        return NULL;
    if (value == 2)
    {
        return StrongholdPieces::generateAndAddPiece(start, pieces, random, mBoundingBox->x0 + offsetX,
                                                     mBoundingBox->y0 + offsetY, mBoundingBox->z0 - 1, direction,
                                                     getGenDepth());
    }
    if (value < 4)
    {
        return StrongholdPieces::generateAndAddPiece(start, pieces, random, mBoundingBox->x0 + offsetX,
                                                     mBoundingBox->y0 + offsetY, mBoundingBox->z1 + 1, direction,
                                                     getGenDepth());
    }
    if (value == 4)
    {
        return StrongholdPieces::generateAndAddPiece(start, pieces, random, mBoundingBox->x0 - 1,
                                                     mBoundingBox->y0 + offsetY, mBoundingBox->z0 + offsetX, direction,
                                                     getGenDepth());
    }
    if (value == 5)
    {
        return StrongholdPieces::generateAndAddPiece(start, pieces, random, mBoundingBox->x1 + 1,
                                                     mBoundingBox->y0 + offsetY, mBoundingBox->z0 + offsetX, direction,
                                                     getGenDepth());
    }
    return NULL;
}

StructurePiece* StrongholdPieces::StrongholdPiece::generateSmallDoorChildLeft(StartPiece* start,
                                                                              std::vector<StructurePiece*>* pieces,
                                                                              Random& random, int offsetY, int offsetXZ)
{
    const Direction* direction = getOrientation();
    if (direction == NULL)
        return NULL;
    unsigned int value = direction->get3DDataValue();
    if (value <= 1)
        return NULL;
    if (value < 4)
    {
        return StrongholdPieces::generateAndAddPiece(start, pieces, random, mBoundingBox->x0 - 1,
                                                     mBoundingBox->y0 + offsetY, mBoundingBox->z0 + offsetXZ,
                                                     Direction::WEST, getGenDepth());
    }
    if (value < 6)
    {
        return StrongholdPieces::generateAndAddPiece(start, pieces, random, mBoundingBox->x0 + offsetXZ,
                                                     mBoundingBox->y0 + offsetY, mBoundingBox->z0 - 1, Direction::NORTH,
                                                     getGenDepth());
    }
    return NULL;
}

StructurePiece* StrongholdPieces::StrongholdPiece::generateSmallDoorChildRight(StartPiece* start,
                                                                               std::vector<StructurePiece*>* pieces,
                                                                               Random& random, int offsetY,
                                                                               int offsetXZ)
{
    const Direction* direction = getOrientation();
    if (direction == NULL)
        return NULL;
    unsigned int value = direction->get3DDataValue();
    if (value <= 1)
        return NULL;
    if (value < 4)
    {
        return StrongholdPieces::generateAndAddPiece(start, pieces, random, mBoundingBox->x1 + 1,
                                                     mBoundingBox->y0 + offsetY, mBoundingBox->z0 + offsetXZ,
                                                     Direction::EAST, getGenDepth());
    }
    if (value < 6)
    {
        return StrongholdPieces::generateAndAddPiece(start, pieces, random, mBoundingBox->x0 + offsetXZ,
                                                     mBoundingBox->y0 + offsetY, mBoundingBox->z1 + 1, Direction::SOUTH,
                                                     getGenDepth());
    }
    return NULL;
}

void StrongholdPieces::resetPieces()
{
    for (std::list<PieceWeight*>::iterator it = sStrongholdPieceWeights.begin(); it != sStrongholdPieceWeights.end();
         ++it)
        delete *it;
    sStrongholdPieceWeights.clear();
    sStrongholdPieceWeights.push_back(new PieceWeight(ePieceClass_Straight, 40, 0));
    sStrongholdPieceWeights.push_back(new PieceWeight(ePieceClass_PrisonHall, 5, 5));
    sStrongholdPieceWeights.push_back(new PieceWeight(ePieceClass_LeftTurn, 20, 0));
    sStrongholdPieceWeights.push_back(new PieceWeight(ePieceClass_RightTurn, 20, 0));
    sStrongholdPieceWeights.push_back(new PieceWeight(ePieceClass_RoomCrossing, 10, 6));
    sStrongholdPieceWeights.push_back(new PieceWeight(ePieceClass_StraightStairsDown, 5, 5));
    sStrongholdPieceWeights.push_back(new PieceWeight(ePieceClass_StairsDown, 5, 5));
    sStrongholdPieceWeights.push_back(new PieceWeight(ePieceClass_FiveCrossing, 5, 4));
    sStrongholdPieceWeights.push_back(new PieceWeight(ePieceClass_ChestCorridor, 5, 4));
    sStrongholdPieceWeights.push_back(new PieceWeight_Library(ePieceClass_Library, 10, 2));
    sStrongholdPieceWeights.push_back(new PieceWeight_PortalRoom(ePieceClass_PortalRoom, 20, 1));
    sForcedPieceClass = ePieceClass_None;
}

void StrongholdPieces::staticCtor()
{
    sStrongholdStoneSelector = new SmoothStoneSelector();
}

unsigned long long StrongholdPieces::loadStatic()
{
    StructureFeatureIO::setPieceId(eStructurePiece_ChestCorridor, &ChestCorridor::Create, L"SHCC");
    StructureFeatureIO::setPieceId(eStructurePiece_FillerCorridor, &FillerCorridor::Create, L"SHFC");
    StructureFeatureIO::setPieceId(eStructurePiece_FiveCrossing, &FiveCrossing::Create, L"SH5C");
    StructureFeatureIO::setPieceId(eStructurePiece_LeftTurn, &LeftTurn::Create, L"SHLT");
    StructureFeatureIO::setPieceId(eStructurePiece_Library, &Library::Create, L"SHLi");
    StructureFeatureIO::setPieceId(eStructurePiece_PortalRoom, &PortalRoom::Create, L"SHPR");
    StructureFeatureIO::setPieceId(eStructurePiece_PrisonHall, &PrisonHall::Create, L"SHPH");
    StructureFeatureIO::setPieceId(eStructurePiece_RightTurn, &RightTurn::Create, L"SHRT");
    StructureFeatureIO::setPieceId(eStructurePiece_StrongholdRoomCrossing, &RoomCrossing::Create, L"SHRC");
    StructureFeatureIO::setPieceId(eStructurePiece_StairsDown, &StairsDown::Create, L"SHSD");
    StructureFeatureIO::setPieceId(eStructurePiece_StrongholdStartPiece, &StairsDown::Create, L"SHStart");
    StructureFeatureIO::setPieceId(eStructurePiece_Straight, &Straight::Create, L"SHS");
    StructureFeatureIO::setPieceId(eStructurePiece_StraightStairsDown, &StraightStairsDown::Create, L"SHSSD");
    return 0;
}

bool StrongholdPieces::updatePieceWeight()
{
    bool canPlace = false;
    sStrongholdWeightTotal = 0;
    for (std::list<PieceWeight*>::iterator it = sStrongholdPieceWeights.begin(); it != sStrongholdPieceWeights.end();
         ++it)
    {
        PieceWeight* weight = *it;
        if (weight->maxPlaceCount > 0 && weight->placeCount < weight->maxPlaceCount)
            canPlace = true;
        sStrongholdWeightTotal += weight->weight;
    }
    return canPlace;
}

StructurePiece* StrongholdPieces::findAndCreatePieceFactory(EPieceClass pieceClass,
                                                            std::vector<StructurePiece*>* pieces, Random& random, int x,
                                                            int y, int z, const Direction* direction, int depth)
{
    switch (pieceClass)
    {
    case ePieceClass_Straight:
        return Straight::createPiece(pieces, random, x, y, z, direction, depth);
    case ePieceClass_PrisonHall:
        return PrisonHall::createPiece(pieces, random, x, y, z, direction, depth);
    case ePieceClass_LeftTurn:
    case ePieceClass_RightTurn:
        return LeftTurn::createPiece(pieces, random, x, y, z, direction, depth);
    case ePieceClass_RoomCrossing:
        return RoomCrossing::createPiece(pieces, random, x, y, z, direction, depth);
    case ePieceClass_StraightStairsDown:
        return StraightStairsDown::createPiece(pieces, random, x, y, z, direction, depth);
    case ePieceClass_StairsDown:
        return StairsDown::createPiece(pieces, random, x, y, z, direction, depth);
    case ePieceClass_FiveCrossing:
        return FiveCrossing::createPiece(pieces, random, x, y, z, direction, depth);
    case ePieceClass_ChestCorridor:
        return ChestCorridor::createPiece(pieces, random, x, y, z, direction, depth);
    case ePieceClass_Library:
        return Library::createPiece(pieces, random, x, y, z, direction, depth);
    case ePieceClass_PortalRoom:
        return PortalRoom::createPiece(pieces, random, x, y, z, direction, depth);
    default:
        return NULL;
    }
}

StructurePiece* StrongholdPieces::generatePieceFromSmallDoor(StartPiece* start, std::vector<StructurePiece*>* pieces,
                                                             Random& random, int x, int y, int z,
                                                             const Direction* direction, int depth)
{
    if (!updatePieceWeight())
        return NULL;

    if (sForcedPieceClass != ePieceClass_None)
    {
        StructurePiece* piece = findAndCreatePieceFactory(sForcedPieceClass, pieces, random, x, y, z, direction, depth);
        sForcedPieceClass = ePieceClass_None;
        if (piece != NULL)
            return piece;
    }

    int attempt = 0;
    while (attempt < 5)
    {
        ++attempt;
        int choice = random.nextInt(sStrongholdWeightTotal);
        for (std::list<PieceWeight*>::iterator it = sStrongholdPieceWeights.begin();
             it != sStrongholdPieceWeights.end(); ++it)
        {
            PieceWeight* weight = *it;
            choice -= weight->weight;
            if (choice < 0)
            {
                if (!weight->doPlace(depth) || weight == start->mLastPieceWeight)
                    break;
                StructurePiece* piece = findAndCreatePieceFactory(weight->pieceClass, pieces, random, x, y, z,
                                                                  direction, depth);
                if (piece != NULL)
                {
                    start->mLastPieceWeight = weight;
                    ++weight->placeCount;
                    if (!weight->isValid())
                    {
                        delete weight;
                        sStrongholdPieceWeights.remove(weight);
                    }
                    return piece;
                }
            }
        }
    }

    BoundingBox* box = FillerCorridor::findPieceBox(pieces, random, x, y, z, direction);
    if (box != NULL)
    {
        if (box->y0 > 1)
            return new FillerCorridor(depth, random, box, direction);
        delete box;
    }
    return NULL;
}

StructurePiece* StrongholdPieces::generateAndAddPiece(StartPiece* start, std::vector<StructurePiece*>* pieces,
                                                      Random& random, int x, int y, int z, const Direction* direction,
                                                      int depth)
{
    if (depth > 50)
        return NULL;
    if (std::abs(x - start->getBoundingBox()->x0) <= 48 && std::abs(z - start->getBoundingBox()->z0) <= 48)
    {
        StructurePiece* piece = generatePieceFromSmallDoor(start, pieces, random, x, y, z, direction, depth + 1);
        if (piece != NULL)
        {
            pieces->push_back(piece);
            start->mPendingChildren.push_back(piece);
        }
        return piece;
    }
    if (start->mLevel->getOriginalSaveVersion() <= 4)
        return NULL;
    if (start->mLevel->getLevelData()->getHasStrongholdEndPortal())
        return NULL;
    for (std::list<PieceWeight*>::iterator it = sStrongholdPieceWeights.begin(); it != sStrongholdPieceWeights.end();
         ++it)
    {
        PieceWeight* weight = *it;
        if (weight->pieceClass == ePieceClass_PortalRoom)
        {
            StructurePiece* piece = PortalRoom::createPiece(pieces, random, x, y, z, direction, depth);
            if (piece != NULL)
            {
                start->mLastPieceWeight = weight;
                ++weight->placeCount;
                if (!weight->isValid())
                {
                    delete weight;
                    sStrongholdPieceWeights.remove(weight);
                }
                return piece;
            }
        }
    }
    return NULL;
}

StrongholdPieces::ChestCorridor::ChestCorridor() : StrongholdPiece()
{
}

StrongholdPieces::ChestCorridor::ChestCorridor(int depth, Random& random, BoundingBox* box, const Direction* direction)
    : StrongholdPiece(depth)
{
    mEntryDoor = randomSmallDoor(random);
    setOrientation(direction);
    mBoundingBox = box;
    mHasPlacedChest = false;
}

StrongholdPieces::FillerCorridor::FillerCorridor() : StrongholdPiece(), mSteps(0)
{
}

StrongholdPieces::FillerCorridor::FillerCorridor(int depth, Random& random, BoundingBox* box,
                                                 const Direction* direction)
    : StrongholdPiece(depth)
{
    if (direction == Direction::NORTH || direction == Direction::SOUTH)
        mSteps = box->getZSpan();
    else
        mSteps = box->getXSpan();
    setOrientation(direction);
    mBoundingBox = box;
}

StrongholdPieces::FiveCrossing::FiveCrossing() : StrongholdPiece()
{
    mLeftLow = mLeftHigh = mRightLow = mRightHigh = false;
}

StrongholdPieces::FiveCrossing::FiveCrossing(int depth, Random& random, BoundingBox* box, const Direction* direction)
    : StrongholdPiece(depth)
{
    mEntryDoor = randomSmallDoor(random);
    setOrientation(direction);
    mBoundingBox = box;
    mLeftLow = random.nextBoolean();
    mLeftHigh = random.nextBoolean();
    mRightLow = random.nextBoolean();
    mRightHigh = random.nextInt(3) > 0;
}

StrongholdPieces::LeftTurn::LeftTurn() : StrongholdPiece()
{
}

StrongholdPieces::LeftTurn::LeftTurn(int depth, Random& random, BoundingBox* box, const Direction* direction)
    : StrongholdPiece(depth)
{
    mEntryDoor = randomSmallDoor(random);
    setOrientation(direction);
    mBoundingBox = box;
}

StrongholdPieces::Library::Library() : StrongholdPiece(), mIsTall(false)
{
}

StrongholdPieces::Library::Library(int depth, Random& random, BoundingBox* box, const Direction* direction)
    : StrongholdPiece(depth),
      mIsTall(box->getYSpan() > 6)
{
    mEntryDoor = randomSmallDoor(random);
    setOrientation(direction);
    mBoundingBox = box;
}

StrongholdPieces::PortalRoom::PortalRoom() : StrongholdPiece()
{
}

StrongholdPieces::PortalRoom::PortalRoom(int depth, Random&, BoundingBox* box, const Direction* direction)
    : StrongholdPiece(depth),
      mHasPlacedSpawner(false)
{
    setOrientation(direction);
    mBoundingBox = box;
}

StrongholdPieces::PrisonHall::PrisonHall() : StrongholdPiece()
{
}

StrongholdPieces::PrisonHall::PrisonHall(int depth, Random& random, BoundingBox* box, const Direction* direction)
    : StrongholdPiece(depth)
{
    mEntryDoor = randomSmallDoor(random);
    setOrientation(direction);
    mBoundingBox = box;
}

StrongholdPieces::RoomCrossing::RoomCrossing() : StrongholdPiece()
{
}

StrongholdPieces::RoomCrossing::RoomCrossing(int depth, Random& random, BoundingBox* box, const Direction* direction)
    : StrongholdPiece(depth),
      mType(random.nextInt(5))
{
    mEntryDoor = randomSmallDoor(random);
    setOrientation(direction);
    mBoundingBox = box;
}

StrongholdPieces::StairsDown::StairsDown() : StrongholdPiece()
{
}

StrongholdPieces::StairsDown::StairsDown(int depth, Random& random, BoundingBox* box, const Direction* direction)
    : StrongholdPiece(depth),
      mIsSource(false)
{
    mEntryDoor = randomSmallDoor(random);
    setOrientation(direction);
    mBoundingBox = box;
}

StrongholdPieces::StairsDown::StairsDown(int depth, Random& random, int x, int z)
    : StrongholdPiece(depth),
      mIsSource(true)
{
    setOrientation(Direction::Plane::HORIZONTAL->getRandomDirection(random));
    mEntryDoor = OPENING;
    if (getOrientation()->getAxis() == Direction::Axis::Z)
        mBoundingBox = new BoundingBox(x, 64, z, x + 5 - 1, 74, z + 5 - 1);
    else
        mBoundingBox = new BoundingBox(x, 64, z, x + 5 - 1, 74, z + 5 - 1);
}

StrongholdPieces::Straight::Straight() : StrongholdPiece()
{
}

StrongholdPieces::Straight::Straight(int depth, Random& random, BoundingBox* box, const Direction* direction)
    : StrongholdPiece(depth),
      mLeftChild(random.nextInt(2) == 0),
      mRightChild(random.nextInt(2) == 0)
{
    mEntryDoor = randomSmallDoor(random);
    setOrientation(direction);
    mBoundingBox = box;
}

StrongholdPieces::StraightStairsDown::StraightStairsDown() : StrongholdPiece()
{
}

StrongholdPieces::StraightStairsDown::StraightStairsDown(int depth, Random& random, BoundingBox* box,
                                                         const Direction* direction)
    : StrongholdPiece(depth)
{
    mEntryDoor = randomSmallDoor(random);
    setOrientation(direction);
    mBoundingBox = box;
}

BoundingBox* StrongholdPieces::FillerCorridor::findPieceBox(std::vector<StructurePiece*>* pieces, Random&, int x, int y,
                                                            int z, const Direction* direction)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -1, -1, 0, 5, 5, 4, direction);
    StructurePiece* collision = StructurePiece::findCollisionPiece(pieces, box);
    if (collision != NULL && collision->getBoundingBox()->y0 == box->y0)
    {
        delete box;
        for (int depth = 3; depth >= 1; --depth)
        {
            BoundingBox* shorter = BoundingBox::orientBox(x, y, z, -1, -1, 0, 5, 5, depth - 1, direction);
            bool blocked = collision->getBoundingBox()->intersects(shorter);
            delete shorter;
            if (!blocked)
                return BoundingBox::orientBox(x, y, z, -1, -1, 0, 5, 5, depth, direction);
        }
        return NULL;
    }
    delete box;
    return NULL;
}

StrongholdPieces::ChestCorridor* StrongholdPieces::ChestCorridor::createPiece(std::vector<StructurePiece*>* pieces,
                                                                              Random& random, int x, int y, int z,
                                                                              const Direction* direction, int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -1, -1, 0, 5, 5, 7, direction);
    StartPiece* start = pieces != NULL ? static_cast<StartPiece*>(pieces->front()) : NULL;
    if (isOkBox(box, start) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new ChestCorridor(depth, random, box, direction);
    delete box;
    return NULL;
}

StrongholdPieces::FiveCrossing* StrongholdPieces::FiveCrossing::createPiece(std::vector<StructurePiece*>* pieces,
                                                                            Random& random, int x, int y, int z,
                                                                            const Direction* direction, int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -4, -3, 0, 10, 9, 11, direction);
    StartPiece* start = pieces != NULL ? static_cast<StartPiece*>(pieces->front()) : NULL;
    if (isOkBox(box, start) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new FiveCrossing(depth, random, box, direction);
    delete box;
    return NULL;
}

StrongholdPieces::LeftTurn* StrongholdPieces::LeftTurn::createPiece(std::vector<StructurePiece*>* pieces,
                                                                    Random& random, int x, int y, int z,
                                                                    const Direction* direction, int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -1, -1, 0, 5, 5, 5, direction);
    StartPiece* start = pieces != NULL ? static_cast<StartPiece*>(pieces->front()) : NULL;
    if (isOkBox(box, start) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new LeftTurn(depth, random, box, direction);
    delete box;
    return NULL;
}

StrongholdPieces::Library* StrongholdPieces::Library::createPiece(std::vector<StructurePiece*>* pieces, Random& random,
                                                                  int x, int y, int z, const Direction* direction,
                                                                  int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -4, -1, 0, 14, 11, 15, direction);
    StartPiece* start = pieces != NULL ? static_cast<StartPiece*>(pieces->front()) : NULL;
    if (!isOkBox(box, start) || StructurePiece::findCollisionPiece(pieces, box) != NULL)
    {
        delete box;
        box = BoundingBox::orientBox(x, y, z, -4, -1, 0, 14, 6, 15, direction);
        if (!isOkBox(box, start) || StructurePiece::findCollisionPiece(pieces, box) != NULL)
        {
            delete box;
            return NULL;
        }
    }
    return new Library(depth, random, box, direction);
}

StrongholdPieces::PortalRoom* StrongholdPieces::PortalRoom::createPiece(std::vector<StructurePiece*>* pieces,
                                                                        Random& random, int x, int y, int z,
                                                                        const Direction* direction, int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -4, -1, 0, 11, 8, 16, direction);
    StartPiece* start = pieces != NULL ? static_cast<StartPiece*>(pieces->front()) : NULL;
    if (isOkBox(box, start) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new PortalRoom(depth, random, box, direction);
    delete box;
    return NULL;
}

StrongholdPieces::PrisonHall* StrongholdPieces::PrisonHall::createPiece(std::vector<StructurePiece*>* pieces,
                                                                        Random& random, int x, int y, int z,
                                                                        const Direction* direction, int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -1, -1, 0, 9, 5, 11, direction);
    StartPiece* start = pieces != NULL ? static_cast<StartPiece*>(pieces->front()) : NULL;
    if (isOkBox(box, start) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new PrisonHall(depth, random, box, direction);
    delete box;
    return NULL;
}

StrongholdPieces::RoomCrossing* StrongholdPieces::RoomCrossing::createPiece(std::vector<StructurePiece*>* pieces,
                                                                            Random& random, int x, int y, int z,
                                                                            const Direction* direction, int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -4, -1, 0, 11, 7, 11, direction);
    StartPiece* start = pieces != NULL ? static_cast<StartPiece*>(pieces->front()) : NULL;
    if (isOkBox(box, start) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new RoomCrossing(depth, random, box, direction);
    delete box;
    return NULL;
}

StrongholdPieces::StairsDown* StrongholdPieces::StairsDown::createPiece(std::vector<StructurePiece*>* pieces,
                                                                        Random& random, int x, int y, int z,
                                                                        const Direction* direction, int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -1, -7, 0, 5, 11, 5, direction);
    StartPiece* start = pieces != NULL ? static_cast<StartPiece*>(pieces->front()) : NULL;
    if (isOkBox(box, start) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new StairsDown(depth, random, box, direction);
    delete box;
    return NULL;
}

StrongholdPieces::Straight* StrongholdPieces::Straight::createPiece(std::vector<StructurePiece*>* pieces,
                                                                    Random& random, int x, int y, int z,
                                                                    const Direction* direction, int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -1, -1, 0, 5, 5, 7, direction);
    StartPiece* start = pieces != NULL ? static_cast<StartPiece*>(pieces->front()) : NULL;
    if (isOkBox(box, start) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new Straight(depth, random, box, direction);
    delete box;
    return NULL;
}

StrongholdPieces::StraightStairsDown* StrongholdPieces::StraightStairsDown::createPiece(
    std::vector<StructurePiece*>* pieces, Random& random, int x, int y, int z, const Direction* direction, int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, -1, -7, 0, 5, 11, 8, direction);
    StartPiece* start = pieces != NULL ? static_cast<StartPiece*>(pieces->front()) : NULL;
    if (isOkBox(box, start) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new StraightStairsDown(depth, random, box, direction);
    delete box;
    return NULL;
}

void StrongholdPieces::ChestCorridor::addAdditionalSaveData(DataOutputStream* output)
{
    StrongholdPiece::addAdditionalSaveData(output);
    output->writeBoolean(mHasPlacedChest);
}

void StrongholdPieces::ChestCorridor::readAdditionalSaveData(DataInputStream* input, StructureManager* manager)
{
    StrongholdPiece::readAdditionalSaveData(input, manager);
    mHasPlacedChest = input->readBoolean();
}

void StrongholdPieces::ChestCorridor::addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces,
                                                  Random& random)
{
    generateSmallDoorChildForward(static_cast<StartPiece*>(start), pieces, random, 1, 1);
}

bool StrongholdPieces::ChestCorridor::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (edgesLiquid(level, box))
        return false;
    generateBox(level, box, 0, 0, 0, 4, 4, 6, true, random, sStrongholdStoneSelector);
    generateSmallDoor(level, random, box, mEntryDoor, 1, 1, 0);
    generateSmallDoor(level, random, box, OPENING, 1, 1, 6);
    generateBox(level, box, 3, 1, 2, 3, 1, 4, Blocks::stonebrick->defaultBlockState(),
                Blocks::stonebrick->defaultBlockState(), false);
    placeBlock(level, Blocks::stone_slab->getBlockState(StoneSlabBlock::Variant::SMOOTHBRICK->getData()), 3, 1, 1, box);
    placeBlock(level, Blocks::stone_slab->getBlockState(StoneSlabBlock::Variant::SMOOTHBRICK->getData()), 3, 1, 5, box);
    placeBlock(level, Blocks::stone_slab->getBlockState(StoneSlabBlock::Variant::SMOOTHBRICK->getData()), 3, 2, 2, box);
    placeBlock(level, Blocks::stone_slab->getBlockState(StoneSlabBlock::Variant::SMOOTHBRICK->getData()), 3, 2, 4, box);
    placeBlock(level, Blocks::stone_slab->getBlockState(StoneSlabBlock::Variant::SMOOTHBRICK->getData()), 2, 1, 2, box);
    placeBlock(level, Blocks::stone_slab->getBlockState(StoneSlabBlock::Variant::SMOOTHBRICK->getData()), 2, 1, 3, box);
    placeBlock(level, Blocks::stone_slab->getBlockState(StoneSlabBlock::Variant::SMOOTHBRICK->getData()), 2, 1, 4, box);
    if (!mHasPlacedChest)
    {
        BlockPos pos(getWorldX(3, 3), getWorldY(2), getWorldZ(3, 3));
        if (box->isInside(pos))
        {
            mHasPlacedChest = true;
            setDirty();
            createChest(level, box, random, 3, 2, 3, &BuiltInLootTables::STRONGHOLD_CORRIDOR);
        }
    }
    return true;
}

StructurePiece* StrongholdPieces::ChestCorridor::Create()
{
    return new ChestCorridor();
}
EStructurePiece StrongholdPieces::ChestCorridor::GetType()
{
    return eStructurePiece_ChestCorridor;
}

void StrongholdPieces::FiveCrossing::addAdditionalSaveData(DataOutputStream* output)
{
    StrongholdPiece::addAdditionalSaveData(output);
    output->writeBoolean(mLeftLow);
    output->writeBoolean(mLeftHigh);
    output->writeBoolean(mRightLow);
    output->writeBoolean(mRightHigh);
}

void StrongholdPieces::FiveCrossing::readAdditionalSaveData(DataInputStream* input, StructureManager* manager)
{
    StrongholdPiece::readAdditionalSaveData(input, manager);
    mLeftLow = input->readBoolean();
    mLeftHigh = input->readBoolean();
    mRightLow = input->readBoolean();
    mRightHigh = input->readBoolean();
}

void StrongholdPieces::FiveCrossing::addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces,
                                                 Random& random)
{
    int lowOffset = 3;
    int highOffset = 5;
    const Direction* orientation = getOrientation();
    if (orientation == Direction::WEST || orientation == Direction::NORTH)
    {
        lowOffset = 8 - lowOffset;
        highOffset = 8 - highOffset;
    }
    StartPiece* startPiece = static_cast<StartPiece*>(start);
    generateSmallDoorChildForward(startPiece, pieces, random, 5, 1);
    if (mLeftLow)
        generateSmallDoorChildLeft(startPiece, pieces, random, lowOffset, 1);
    if (mLeftHigh)
        generateSmallDoorChildLeft(startPiece, pieces, random, highOffset, 7);
    if (mRightLow)
        generateSmallDoorChildRight(startPiece, pieces, random, lowOffset, 1);
    if (mRightHigh)
        generateSmallDoorChildRight(startPiece, pieces, random, highOffset, 7);
}

bool StrongholdPieces::FiveCrossing::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (edgesLiquid(level, box))
        return false;
    generateBox(level, box, 0, 0, 0, 9, 8, 10, true, random, sStrongholdStoneSelector);
    generateSmallDoor(level, random, box, mEntryDoor, 4, 3, 0);
    if (mLeftLow)
        generateBox(level, box, 0, 3, 1, 0, 5, 3, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                    false);
    if (mRightLow)
        generateBox(level, box, 9, 3, 1, 9, 5, 3, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                    false);
    if (mLeftHigh)
        generateBox(level, box, 0, 5, 7, 0, 7, 9, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                    false);
    if (mRightHigh)
        generateBox(level, box, 9, 5, 7, 9, 7, 9, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                    false);
    generateBox(level, box, 5, 1, 10, 7, 3, 10, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 1, 2, 1, 8, 2, 6, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 4, 1, 5, 4, 4, 9, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 8, 1, 5, 8, 4, 9, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 1, 4, 7, 3, 4, 9, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 1, 3, 5, 3, 3, 6, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 1, 3, 4, 3, 3, 4, Blocks::stone_slab->defaultBlockState(),
                Blocks::stone_slab->defaultBlockState(), false);
    generateBox(level, box, 1, 4, 6, 3, 4, 6, Blocks::stone_slab->defaultBlockState(),
                Blocks::stone_slab->defaultBlockState(), false);
    generateBox(level, box, 5, 1, 7, 7, 1, 8, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 5, 1, 9, 7, 1, 9, Blocks::stone_slab->defaultBlockState(),
                Blocks::stone_slab->defaultBlockState(), false);
    generateBox(level, box, 5, 2, 7, 7, 2, 7, Blocks::stone_slab->defaultBlockState(),
                Blocks::stone_slab->defaultBlockState(), false);
    generateBox(level, box, 4, 5, 7, 4, 5, 9, Blocks::stone_slab->defaultBlockState(),
                Blocks::stone_slab->defaultBlockState(), false);
    generateBox(level, box, 8, 5, 7, 8, 5, 9, Blocks::stone_slab->defaultBlockState(),
                Blocks::stone_slab->defaultBlockState(), false);
    generateBox(level, box, 5, 5, 7, 7, 5, 9, Blocks::double_stone_slab->defaultBlockState(),
                Blocks::double_stone_slab->defaultBlockState(), false);
    placeBlock(level, Blocks::torch->defaultBlockState()->setValue(TorchBlock::FACING, Direction::SOUTH), 6, 5, 6, box);
    return true;
}

StructurePiece* StrongholdPieces::FiveCrossing::Create()
{
    return new FiveCrossing();
}
EStructurePiece StrongholdPieces::FiveCrossing::GetType()
{
    return eStructurePiece_FiveCrossing;
}

void StrongholdPieces::LeftTurn::addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces,
                                             Random& random)
{
    const Direction* orientation = getOrientation();
    if (orientation == Direction::NORTH || orientation == Direction::EAST)
        generateSmallDoorChildLeft(static_cast<StartPiece*>(start), pieces, random, 1, 1);
    else
        generateSmallDoorChildRight(static_cast<StartPiece*>(start), pieces, random, 1, 1);
}

bool StrongholdPieces::LeftTurn::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (edgesLiquid(level, box))
        return false;
    generateBox(level, box, 0, 0, 0, 4, 4, 4, true, random, sStrongholdStoneSelector);
    generateSmallDoor(level, random, box, mEntryDoor, 1, 1, 0);
    const Direction* orientation = getOrientation();
    if (orientation == Direction::NORTH || orientation == Direction::EAST)
    {
        generateBox(level, box, 0, 1, 1, 0, 3, 3, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                    false);
    }
    else
    {
        generateBox(level, box, 4, 1, 1, 4, 3, 3, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                    false);
    }
    return true;
}

StructurePiece* StrongholdPieces::LeftTurn::Create()
{
    return new LeftTurn();
}
EStructurePiece StrongholdPieces::LeftTurn::GetType()
{
    return eStructurePiece_LeftTurn;
}

void StrongholdPieces::Library::addAdditionalSaveData(DataOutputStream* output)
{
    StrongholdPiece::addAdditionalSaveData(output);
    output->writeBoolean(mIsTall);
}

void StrongholdPieces::Library::readAdditionalSaveData(DataInputStream* input, StructureManager* manager)
{
    StrongholdPiece::readAdditionalSaveData(input, manager);
    mIsTall = input->readBoolean();
}

bool StrongholdPieces::Library::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (edgesLiquid(level, box))
        return false;
    int height = 11;
    if (!mIsTall)
        height = 6;
    generateBox(level, box, 0, 0, 0, 13, height - 1, 14, true, random, sStrongholdStoneSelector);
    generateSmallDoor(level, random, box, mEntryDoor, 4, 1, 0);
    generateMaybeBox(level, box, random, 0.07f, 2, 1, 1, 11, 4, 13, Blocks::web->defaultBlockState(),
                     Blocks::web->defaultBlockState(), false, false);

    for (int z = 1; z <= 13; ++z)
    {
        if ((z - 1) % 4 == 0)
        {
            generateBox(level, box, 1, 1, z, 1, 4, z, Blocks::planks->defaultBlockState(),
                        Blocks::planks->defaultBlockState(), false);
            generateBox(level, box, 12, 1, z, 12, 4, z, Blocks::planks->defaultBlockState(),
                        Blocks::planks->defaultBlockState(), false);
            placeBlock(level, Blocks::torch->defaultBlockState()->setValue(TorchBlock::FACING, Direction::EAST), 2, 3,
                       z, box);
            placeBlock(level, Blocks::torch->defaultBlockState()->setValue(TorchBlock::FACING, Direction::WEST), 11, 3,
                       z, box);
            if (mIsTall)
            {
                generateBox(level, box, 1, 6, z, 1, 9, z, Blocks::planks->defaultBlockState(),
                            Blocks::planks->defaultBlockState(), false);
                generateBox(level, box, 12, 6, z, 12, 9, z, Blocks::planks->defaultBlockState(),
                            Blocks::planks->defaultBlockState(), false);
            }
        }
        else
        {
            generateBox(level, box, 1, 1, z, 1, 4, z, Blocks::bookshelf->defaultBlockState(),
                        Blocks::bookshelf->defaultBlockState(), false);
            generateBox(level, box, 12, 1, z, 12, 4, z, Blocks::bookshelf->defaultBlockState(),
                        Blocks::bookshelf->defaultBlockState(), false);
            if (mIsTall)
            {
                generateBox(level, box, 1, 6, z, 1, 9, z, Blocks::bookshelf->defaultBlockState(),
                            Blocks::bookshelf->defaultBlockState(), false);
                generateBox(level, box, 12, 6, z, 12, 9, z, Blocks::bookshelf->defaultBlockState(),
                            Blocks::bookshelf->defaultBlockState(), false);
            }
        }
    }

    for (int z = 3; z < 12; z += 2)
    {
        generateBox(level, box, 3, 1, z, 4, 3, z, Blocks::bookshelf->defaultBlockState(),
                    Blocks::bookshelf->defaultBlockState(), false);
        generateBox(level, box, 6, 1, z, 7, 3, z, Blocks::bookshelf->defaultBlockState(),
                    Blocks::bookshelf->defaultBlockState(), false);
        generateBox(level, box, 9, 1, z, 10, 3, z, Blocks::bookshelf->defaultBlockState(),
                    Blocks::bookshelf->defaultBlockState(), false);
    }

    if (mIsTall)
    {
        generateBox(level, box, 1, 5, 1, 3, 5, 13, Blocks::planks->defaultBlockState(),
                    Blocks::planks->defaultBlockState(), false);
        generateBox(level, box, 10, 5, 1, 12, 5, 13, Blocks::planks->defaultBlockState(),
                    Blocks::planks->defaultBlockState(), false);
        generateBox(level, box, 4, 5, 1, 9, 5, 2, Blocks::planks->defaultBlockState(),
                    Blocks::planks->defaultBlockState(), false);
        generateBox(level, box, 4, 5, 12, 9, 5, 13, Blocks::planks->defaultBlockState(),
                    Blocks::planks->defaultBlockState(), false);
        placeBlock(level, Blocks::planks->defaultBlockState(), 9, 5, 11, box);
        placeBlock(level, Blocks::planks->defaultBlockState(), 8, 5, 11, box);
        placeBlock(level, Blocks::planks->defaultBlockState(), 9, 5, 10, box);
        generateBox(level, box, 3, 6, 2, 3, 6, 12, Blocks::fence->defaultBlockState(),
                    Blocks::fence->defaultBlockState(), false);
        generateBox(level, box, 10, 6, 2, 10, 6, 10, Blocks::fence->defaultBlockState(),
                    Blocks::fence->defaultBlockState(), false);
        generateBox(level, box, 4, 6, 2, 9, 6, 2, Blocks::fence->defaultBlockState(),
                    Blocks::fence->defaultBlockState(), false);
        generateBox(level, box, 4, 6, 12, 8, 6, 12, Blocks::fence->defaultBlockState(),
                    Blocks::fence->defaultBlockState(), false);
        placeBlock(level, Blocks::fence->defaultBlockState(), 9, 6, 11, box);
        placeBlock(level, Blocks::fence->defaultBlockState(), 8, 6, 11, box);
        placeBlock(level, Blocks::fence->defaultBlockState(), 9, 6, 10, box);

        const BlockState* ladder = Blocks::ladder->defaultBlockState()->setValue(LadderBlock::FACING, Direction::SOUTH);
        placeBlock(level, ladder, 10, 1, 13, box);
        placeBlock(level, ladder, 10, 2, 13, box);
        placeBlock(level, ladder, 10, 3, 13, box);
        placeBlock(level, ladder, 10, 4, 13, box);
        placeBlock(level, ladder, 10, 5, 13, box);
        placeBlock(level, ladder, 10, 6, 13, box);
        placeBlock(level, ladder, 10, 7, 13, box);

        placeBlock(level, Blocks::fence->defaultBlockState(), 6, 9, 7, box);
        placeBlock(level, Blocks::fence->defaultBlockState(), 7, 9, 7, box);
        placeBlock(level, Blocks::fence->defaultBlockState(), 6, 8, 7, box);
        placeBlock(level, Blocks::fence->defaultBlockState(), 7, 8, 7, box);
        placeBlock(level, Blocks::fence->defaultBlockState(), 6, 7, 7, box);
        placeBlock(level, Blocks::fence->defaultBlockState(), 7, 7, 7, box);
        placeBlock(level, Blocks::fence->defaultBlockState(), 5, 7, 7, box);
        placeBlock(level, Blocks::fence->defaultBlockState(), 8, 7, 7, box);
        placeBlock(level, Blocks::fence->defaultBlockState(), 6, 7, 6, box);
        placeBlock(level, Blocks::fence->defaultBlockState(), 6, 7, 8, box);
        placeBlock(level, Blocks::fence->defaultBlockState(), 7, 7, 6, box);
        placeBlock(level, Blocks::fence->defaultBlockState(), 7, 7, 8, box);

        const BlockState* torch = Blocks::torch->defaultBlockState()->setValue(TorchBlock::FACING, Direction::UP);
        placeBlock(level, torch, 5, 8, 7, box);
        placeBlock(level, torch, 8, 8, 7, box);
        placeBlock(level, torch, 6, 8, 6, box);
        placeBlock(level, torch, 6, 8, 8, box);
        placeBlock(level, torch, 7, 8, 6, box);
        placeBlock(level, torch, 7, 8, 8, box);
    }

    createChest(level, box, random, 3, 3, 5, &BuiltInLootTables::STRONGHOLD_LIBRARY);
    if (mIsTall)
    {
        placeBlock(level, Blocks::air->defaultBlockState(), 12, 9, 1, box);
        createChest(level, box, random, 12, 8, 1, &BuiltInLootTables::STRONGHOLD_LIBRARY);
    }
    return true;
}

StructurePiece* StrongholdPieces::Library::Create()
{
    return new Library();
}
EStructurePiece StrongholdPieces::Library::GetType()
{
    return eStructurePiece_Library;
}

void StrongholdPieces::PrisonHall::addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces,
                                               Random& random)
{
    generateSmallDoorChildForward(static_cast<StartPiece*>(start), pieces, random, 1, 1);
}

bool StrongholdPieces::PrisonHall::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (edgesLiquid(level, box))
        return false;
    generateBox(level, box, 0, 0, 0, 8, 4, 10, true, random, sStrongholdStoneSelector);
    generateSmallDoor(level, random, box, mEntryDoor, 1, 1, 0);
    generateBox(level, box, 1, 1, 10, 3, 3, 10, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 4, 1, 1, 4, 3, 1, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 4, 1, 3, 4, 3, 3, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 4, 1, 7, 4, 3, 7, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 4, 1, 9, 4, 3, 9, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 4, 1, 4, 4, 3, 6, Blocks::iron_bars->defaultBlockState(),
                Blocks::iron_bars->defaultBlockState(), false);
    generateBox(level, box, 5, 1, 5, 7, 3, 5, Blocks::iron_bars->defaultBlockState(),
                Blocks::iron_bars->defaultBlockState(), false);
    placeBlock(level, Blocks::iron_bars->defaultBlockState(), 4, 3, 2, box);
    placeBlock(level, Blocks::iron_bars->defaultBlockState(), 4, 3, 8, box);

    const BlockState* lowerDoor = Blocks::iron_door->defaultBlockState()->setValue(DoorBlock::FACING, Direction::WEST);
    const BlockState* upperDoor = Blocks::iron_door->defaultBlockState()
                                      ->setValue(DoorBlock::FACING, Direction::WEST)
                                      ->setValue(DoorBlock::HALF, DoorBlock::UPPER);
    placeBlock(level, lowerDoor, 4, 1, 2, box);
    placeBlock(level, upperDoor, 4, 2, 2, box);
    placeBlock(level, lowerDoor, 4, 1, 8, box);
    placeBlock(level, upperDoor, 4, 2, 8, box);
    return true;
}

StructurePiece* StrongholdPieces::PrisonHall::Create()
{
    return new PrisonHall();
}
EStructurePiece StrongholdPieces::PrisonHall::GetType()
{
    return eStructurePiece_PrisonHall;
}

void StrongholdPieces::RoomCrossing::addAdditionalSaveData(DataOutputStream* output)
{
    StrongholdPiece::addAdditionalSaveData(output);
    output->writeInt(mType);
}

void StrongholdPieces::RoomCrossing::readAdditionalSaveData(DataInputStream* input, StructureManager* manager)
{
    StrongholdPiece::readAdditionalSaveData(input, manager);
    mType = input->readInt();
}

void StrongholdPieces::RoomCrossing::addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces,
                                                 Random& random)
{
    StartPiece* startPiece = static_cast<StartPiece*>(start);
    generateSmallDoorChildForward(startPiece, pieces, random, 4, 1);
    generateSmallDoorChildLeft(startPiece, pieces, random, 1, 4);
    generateSmallDoorChildRight(startPiece, pieces, random, 1, 4);
}

bool StrongholdPieces::RoomCrossing::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (edgesLiquid(level, box))
        return false;
    generateBox(level, box, 0, 0, 0, 10, 6, 10, true, random, sStrongholdStoneSelector);
    generateSmallDoor(level, random, box, mEntryDoor, 4, 1, 0);
    generateBox(level, box, 4, 1, 10, 6, 3, 10, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 0, 1, 4, 0, 3, 6, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 10, 1, 4, 10, 3, 6, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);

    switch (mType)
    {
    case 0:
    {
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 5, 1, 5, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 5, 2, 5, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 5, 3, 5, box);
        placeBlock(level, Blocks::torch->defaultBlockState()->setValue(TorchBlock::FACING, Direction::WEST), 4, 3, 5,
                   box);
        placeBlock(level, Blocks::torch->defaultBlockState()->setValue(TorchBlock::FACING, Direction::EAST), 6, 3, 5,
                   box);
        placeBlock(level, Blocks::torch->defaultBlockState()->setValue(TorchBlock::FACING, Direction::SOUTH), 5, 3, 4,
                   box);
        placeBlock(level, Blocks::torch->defaultBlockState()->setValue(TorchBlock::FACING, Direction::NORTH), 5, 3, 6,
                   box);
        placeBlock(level, Blocks::stone_slab->defaultBlockState(), 4, 1, 4, box);
        placeBlock(level, Blocks::stone_slab->defaultBlockState(), 4, 1, 5, box);
        placeBlock(level, Blocks::stone_slab->defaultBlockState(), 4, 1, 6, box);
        placeBlock(level, Blocks::stone_slab->defaultBlockState(), 6, 1, 4, box);
        placeBlock(level, Blocks::stone_slab->defaultBlockState(), 6, 1, 5, box);
        placeBlock(level, Blocks::stone_slab->defaultBlockState(), 6, 1, 6, box);
        placeBlock(level, Blocks::stone_slab->defaultBlockState(), 5, 1, 4, box);
        placeBlock(level, Blocks::stone_slab->defaultBlockState(), 5, 1, 6, box);
        break;
    }
    case 1:
        for (int i = 0; i < 5; ++i)
        {
            placeBlock(level, Blocks::stonebrick->defaultBlockState(), 3, 1, 3 + i, box);
            placeBlock(level, Blocks::stonebrick->defaultBlockState(), 7, 1, 3 + i, box);
            placeBlock(level, Blocks::stonebrick->defaultBlockState(), 3 + i, 1, 3, box);
            placeBlock(level, Blocks::stonebrick->defaultBlockState(), 3 + i, 1, 7, box);
        }
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 5, 1, 5, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 5, 2, 5, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 5, 3, 5, box);
        placeBlock(level, Blocks::flowing_water->defaultBlockState(), 5, 4, 5, box);
        break;
    case 2:
    {
        for (int z = 1; z <= 9; ++z)
        {
            placeBlock(level, Blocks::cobblestone->defaultBlockState(), 1, 3, z, box);
            placeBlock(level, Blocks::cobblestone->defaultBlockState(), 9, 3, z, box);
        }
        for (int x = 1; x <= 9; ++x)
        {
            placeBlock(level, Blocks::cobblestone->defaultBlockState(), x, 3, 1, box);
            placeBlock(level, Blocks::cobblestone->defaultBlockState(), x, 3, 9, box);
        }
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 5, 1, 4, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 5, 1, 6, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 5, 3, 4, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 5, 3, 6, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 4, 1, 5, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 6, 1, 5, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 4, 3, 5, box);
        placeBlock(level, Blocks::cobblestone->defaultBlockState(), 6, 3, 5, box);
        for (int y = 1; y <= 3; ++y)
        {
            placeBlock(level, Blocks::cobblestone->defaultBlockState(), 4, y, 4, box);
            placeBlock(level, Blocks::cobblestone->defaultBlockState(), 6, y, 4, box);
            placeBlock(level, Blocks::cobblestone->defaultBlockState(), 4, y, 6, box);
            placeBlock(level, Blocks::cobblestone->defaultBlockState(), 6, y, 6, box);
        }
        placeBlock(level, Blocks::torch->defaultBlockState(), 5, 3, 5, box);
        for (int z = 2; z <= 8; ++z)
        {
            placeBlock(level, Blocks::planks->defaultBlockState(), 2, 3, z, box);
            placeBlock(level, Blocks::planks->defaultBlockState(), 3, 3, z, box);
            if (z <= 3 || z >= 7)
            {
                placeBlock(level, Blocks::planks->defaultBlockState(), 4, 3, z, box);
                placeBlock(level, Blocks::planks->defaultBlockState(), 5, 3, z, box);
                placeBlock(level, Blocks::planks->defaultBlockState(), 6, 3, z, box);
            }
            placeBlock(level, Blocks::planks->defaultBlockState(), 7, 3, z, box);
            placeBlock(level, Blocks::planks->defaultBlockState(), 8, 3, z, box);
        }
        const BlockState* ladder = Blocks::ladder->defaultBlockState()->setValue(LadderBlock::FACING, Direction::WEST);
        placeBlock(level, ladder, 9, 1, 3, box);
        placeBlock(level, ladder, 9, 2, 3, box);
        placeBlock(level, ladder, 9, 3, 3, box);
        createChest(level, box, random, 3, 4, 8, &BuiltInLootTables::STRONGHOLD_CROSSING);
        break;
    }
    }
    return true;
}

StructurePiece* StrongholdPieces::RoomCrossing::Create()
{
    return new RoomCrossing();
}
EStructurePiece StrongholdPieces::RoomCrossing::GetType()
{
    return eStructurePiece_StrongholdRoomCrossing;
}

void StrongholdPieces::StairsDown::addAdditionalSaveData(DataOutputStream* output)
{
    StrongholdPiece::addAdditionalSaveData(output);
    output->writeBoolean(mIsSource);
}

void StrongholdPieces::StairsDown::readAdditionalSaveData(DataInputStream* input, StructureManager* manager)
{
    StrongholdPiece::readAdditionalSaveData(input, manager);
    mIsSource = input->readBoolean();
}

void StrongholdPieces::StairsDown::addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces,
                                               Random& random)
{
    if (mIsSource)
        sForcedPieceClass = ePieceClass_FiveCrossing;
    generateSmallDoorChildForward(static_cast<StartPiece*>(start), pieces, random, 1, 1);
}

bool StrongholdPieces::StairsDown::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (edgesLiquid(level, box))
        return false;
    generateBox(level, box, 0, 0, 0, 4, 10, 4, true, random, sStrongholdStoneSelector);
    generateSmallDoor(level, random, box, mEntryDoor, 1, 7, 0);
    generateSmallDoor(level, random, box, OPENING, 1, 1, 4);
    placeBlock(level, Blocks::stonebrick->defaultBlockState(), 2, 6, 1, box);
    placeBlock(level, Blocks::stonebrick->defaultBlockState(), 1, 5, 1, box);
    placeBlock(level, Blocks::stone_slab->getBlockState(StoneSlabBlock::Variant::STONE->getData()), 1, 6, 1, box);
    placeBlock(level, Blocks::stonebrick->defaultBlockState(), 1, 5, 2, box);
    placeBlock(level, Blocks::stonebrick->defaultBlockState(), 1, 4, 3, box);
    placeBlock(level, Blocks::stone_slab->getBlockState(StoneSlabBlock::Variant::STONE->getData()), 1, 5, 3, box);
    placeBlock(level, Blocks::stonebrick->defaultBlockState(), 2, 4, 3, box);
    placeBlock(level, Blocks::stonebrick->defaultBlockState(), 3, 3, 3, box);
    placeBlock(level, Blocks::stone_slab->getBlockState(StoneSlabBlock::Variant::STONE->getData()), 3, 4, 3, box);
    placeBlock(level, Blocks::stonebrick->defaultBlockState(), 3, 3, 2, box);
    placeBlock(level, Blocks::stonebrick->defaultBlockState(), 3, 2, 1, box);
    placeBlock(level, Blocks::stone_slab->getBlockState(StoneSlabBlock::Variant::STONE->getData()), 3, 3, 1, box);
    placeBlock(level, Blocks::stonebrick->defaultBlockState(), 2, 2, 1, box);
    placeBlock(level, Blocks::stonebrick->defaultBlockState(), 1, 1, 1, box);
    placeBlock(level, Blocks::stone_slab->getBlockState(StoneSlabBlock::Variant::STONE->getData()), 1, 2, 1, box);
    placeBlock(level, Blocks::stonebrick->defaultBlockState(), 1, 1, 2, box);
    placeBlock(level, Blocks::stone_slab->getBlockState(StoneSlabBlock::Variant::STONE->getData()), 1, 1, 3, box);
    return true;
}

StructurePiece* StrongholdPieces::StairsDown::Create()
{
    return new StairsDown();
}
EStructurePiece StrongholdPieces::StairsDown::GetType()
{
    return eStructurePiece_StairsDown;
}

void StrongholdPieces::Straight::addAdditionalSaveData(DataOutputStream* output)
{
    StrongholdPiece::addAdditionalSaveData(output);
    output->writeBoolean(mLeftChild);
    output->writeBoolean(mRightChild);
}

void StrongholdPieces::Straight::readAdditionalSaveData(DataInputStream* input, StructureManager* manager)
{
    StrongholdPiece::readAdditionalSaveData(input, manager);
    mLeftChild = input->readBoolean();
    mRightChild = input->readBoolean();
}

void StrongholdPieces::Straight::addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces,
                                             Random& random)
{
    StartPiece* startPiece = static_cast<StartPiece*>(start);
    generateSmallDoorChildForward(startPiece, pieces, random, 1, 1);
    if (mLeftChild)
        generateSmallDoorChildLeft(startPiece, pieces, random, 1, 2);
    if (mRightChild)
        generateSmallDoorChildRight(startPiece, pieces, random, 1, 2);
}

bool StrongholdPieces::Straight::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (edgesLiquid(level, box))
        return false;
    generateBox(level, box, 0, 0, 0, 4, 4, 6, true, random, sStrongholdStoneSelector);
    generateSmallDoor(level, random, box, mEntryDoor, 1, 1, 0);
    generateSmallDoor(level, random, box, OPENING, 1, 1, 6);
    const BlockState* torchEast = Blocks::torch->defaultBlockState()->setValue(TorchBlock::FACING, Direction::EAST);
    const BlockState* torchWest = Blocks::torch->defaultBlockState()->setValue(TorchBlock::FACING, Direction::WEST);
    maybeGenerateBlock(level, box, random, 0.1f, 1, 2, 1, torchEast);
    maybeGenerateBlock(level, box, random, 0.1f, 3, 2, 1, torchWest);
    maybeGenerateBlock(level, box, random, 0.1f, 1, 2, 5, torchEast);
    maybeGenerateBlock(level, box, random, 0.1f, 3, 2, 5, torchWest);
    if (mLeftChild)
    {
        generateBox(level, box, 0, 1, 2, 0, 3, 4, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                    false);
    }
    if (mRightChild)
    {
        generateBox(level, box, 4, 1, 2, 4, 3, 4, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                    false);
    }
    return true;
}

StructurePiece* StrongholdPieces::Straight::Create()
{
    return new Straight();
}
EStructurePiece StrongholdPieces::Straight::GetType()
{
    return eStructurePiece_Straight;
}

void StrongholdPieces::StraightStairsDown::addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces,
                                                       Random& random)
{
    generateSmallDoorChildForward(static_cast<StartPiece*>(start), pieces, random, 1, 1);
}

bool StrongholdPieces::StraightStairsDown::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (edgesLiquid(level, box))
        return false;
    generateBox(level, box, 0, 0, 0, 4, 10, 7, true, random, sStrongholdStoneSelector);
    generateSmallDoor(level, random, box, mEntryDoor, 1, 7, 0);
    generateSmallDoor(level, random, box, OPENING, 1, 1, 7);
    const BlockState* stairs = Blocks::stone_stairs->defaultBlockState()->setValue(StairBlock::FACING,
                                                                                   Direction::SOUTH);
    for (int i = 0; i < 6; ++i)
    {
        placeBlock(level, stairs, 1, 6 - i, 1 + i, box);
        placeBlock(level, stairs, 2, 6 - i, 1 + i, box);
        placeBlock(level, stairs, 3, 6 - i, 1 + i, box);
        if (i < 5)
        {
            placeBlock(level, Blocks::stonebrick->defaultBlockState(), 1, 5 - i, 1 + i, box);
            placeBlock(level, Blocks::stonebrick->defaultBlockState(), 2, 5 - i, 1 + i, box);
            placeBlock(level, Blocks::stonebrick->defaultBlockState(), 3, 5 - i, 1 + i, box);
        }
    }
    return true;
}

StructurePiece* StrongholdPieces::StraightStairsDown::Create()
{
    return new StraightStairsDown();
}
EStructurePiece StrongholdPieces::StraightStairsDown::GetType()
{
    return eStructurePiece_StraightStairsDown;
}

void StrongholdPieces::PortalRoom::addAdditionalSaveData(DataOutputStream* output)
{
    StrongholdPiece::addAdditionalSaveData(output);
    output->writeBoolean(mHasPlacedSpawner);
}

void StrongholdPieces::PortalRoom::readAdditionalSaveData(DataInputStream* input, StructureManager* manager)
{
    StrongholdPiece::readAdditionalSaveData(input, manager);
    mHasPlacedSpawner = input->readBoolean();
}

void StrongholdPieces::PortalRoom::addChildren(StructurePiece* start, std::vector<StructurePiece*>*, Random&)
{
    if (start != NULL)
        static_cast<StartPiece*>(start)->mPortalRoom = this;
}

bool StrongholdPieces::PortalRoom::postProcess(Level* level, Random& random, BoundingBox* box)
{
    generateBox(level, box, 0, 0, 0, 10, 7, 15, false, random, sStrongholdStoneSelector);
    generateSmallDoor(level, random, box, GRATES, 4, 1, 0);
    generateBox(level, box, 1, 6, 1, 1, 6, 14, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 9, 6, 1, 9, 6, 14, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 2, 6, 1, 8, 6, 2, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 2, 6, 14, 8, 6, 14, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 1, 1, 1, 2, 1, 4, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 8, 1, 1, 9, 1, 4, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 1, 1, 1, 1, 1, 3, Blocks::flowing_lava->defaultBlockState(),
                Blocks::flowing_lava->defaultBlockState(), false);
    generateBox(level, box, 9, 1, 1, 9, 1, 3, Blocks::flowing_lava->defaultBlockState(),
                Blocks::flowing_lava->defaultBlockState(), false);
    generateBox(level, box, 3, 1, 8, 7, 1, 12, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 4, 1, 9, 6, 1, 11, Blocks::flowing_lava->defaultBlockState(),
                Blocks::flowing_lava->defaultBlockState(), false);
    for (int z = 3; z < 14; z += 2)
    {
        generateBox(level, box, 0, 3, z, 0, 4, z, Blocks::iron_bars->defaultBlockState(),
                    Blocks::iron_bars->defaultBlockState(), false);
        generateBox(level, box, 10, 3, z, 10, 4, z, Blocks::iron_bars->defaultBlockState(),
                    Blocks::iron_bars->defaultBlockState(), false);
    }
    for (int x = 2; x < 9; x += 2)
    {
        generateBox(level, box, x, 3, 15, x, 4, 15, Blocks::iron_bars->defaultBlockState(),
                    Blocks::iron_bars->defaultBlockState(), false);
    }

    const BlockState* stairs = Blocks::stone_brick_stairs->defaultBlockState()->setValue(StairBlock::FACING,
                                                                                         Direction::NORTH);
    generateBox(level, box, 4, 1, 5, 6, 1, 7, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 4, 2, 6, 6, 2, 7, false, random, sStrongholdStoneSelector);
    generateBox(level, box, 4, 3, 7, 6, 3, 7, false, random, sStrongholdStoneSelector);
    for (int x = 4; x <= 6; ++x)
    {
        placeBlock(level, stairs, x, 1, 4, box);
        placeBlock(level, stairs, x, 2, 5, box);
        placeBlock(level, stairs, x, 3, 6, box);
    }

    const BlockState* frameNorthState = Blocks::end_portal_frame->defaultBlockState()->setValue(
        EndPortalFrameBlock::FACING, Direction::NORTH);
    const BlockState* frameSouthState = Blocks::end_portal_frame->defaultBlockState()->setValue(
        EndPortalFrameBlock::FACING, Direction::SOUTH);
    const BlockState* frameEastState = Blocks::end_portal_frame->defaultBlockState()->setValue(
        EndPortalFrameBlock::FACING, Direction::EAST);
    const BlockState* frameWestState = Blocks::end_portal_frame->defaultBlockState()->setValue(
        EndPortalFrameBlock::FACING, Direction::WEST);

    bool complete = true;
    bool eyes[12];
    for (int i = 0; i < 12; ++i)
    {
        eyes[i] = random.nextFloat() > 0.9f;
        complete &= eyes[i];
    }

    placeBlock(level, frameNorthState->setValue(EndPortalFrameBlock::EYE, eyes[0]), 4, 3, 8, box);
    placeBlock(level, frameNorthState->setValue(EndPortalFrameBlock::EYE, eyes[1]), 5, 3, 8, box);
    placeBlock(level, frameNorthState->setValue(EndPortalFrameBlock::EYE, eyes[2]), 6, 3, 8, box);
    placeBlock(level, frameSouthState->setValue(EndPortalFrameBlock::EYE, eyes[3]), 4, 3, 12, box);
    placeBlock(level, frameSouthState->setValue(EndPortalFrameBlock::EYE, eyes[4]), 5, 3, 12, box);
    placeBlock(level, frameSouthState->setValue(EndPortalFrameBlock::EYE, eyes[5]), 6, 3, 12, box);
    placeBlock(level, frameEastState->setValue(EndPortalFrameBlock::EYE, eyes[6]), 3, 3, 9, box);
    placeBlock(level, frameEastState->setValue(EndPortalFrameBlock::EYE, eyes[7]), 3, 3, 10, box);
    placeBlock(level, frameEastState->setValue(EndPortalFrameBlock::EYE, eyes[8]), 3, 3, 11, box);
    placeBlock(level, frameWestState->setValue(EndPortalFrameBlock::EYE, eyes[9]), 7, 3, 9, box);
    placeBlock(level, frameWestState->setValue(EndPortalFrameBlock::EYE, eyes[10]), 7, 3, 10, box);
    placeBlock(level, frameWestState->setValue(EndPortalFrameBlock::EYE, eyes[11]), 7, 3, 11, box);

    if (complete)
    {
        const BlockState* portal = Blocks::end_portal->defaultBlockState();
        placeBlock(level, portal, 4, 3, 9, box);
        placeBlock(level, portal, 5, 3, 9, box);
        placeBlock(level, portal, 6, 3, 9, box);
        placeBlock(level, portal, 4, 3, 10, box);
        placeBlock(level, portal, 5, 3, 10, box);
        placeBlock(level, portal, 6, 3, 10, box);
        placeBlock(level, portal, 4, 3, 11, box);
        placeBlock(level, portal, 5, 3, 11, box);
        placeBlock(level, portal, 6, 3, 11, box);
    }

    if (!mHasPlacedSpawner)
    {
        int worldY = getWorldY(3);
        int worldX = getWorldX(5, 6);
        int worldZ = getWorldZ(5, 6);
        BlockPos pos(worldX, worldY, worldZ);
        if (box->isInside(pos))
        {
            gApp.AddTerrainFeaturePosition((_eTerrainFeatureType)6, pos.x, pos.z);
            level->getLevelData()->setXStrongholdEndPortal(pos.x);
            level->getLevelData()->setZStrongholdEndPortal(pos.z);
            level->getLevelData()->setHasStrongholdEndPortal();
            setDirty();
            mHasPlacedSpawner = true;
            level->setBlock(pos, Blocks::mob_spawner->defaultBlockState(), Block::UPDATE_CLIENTS, false);
            boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
            bool isSpawner = eTYPE_DERIVED_FROM(BlockEntityType::MOB_SPAWNER, blockEntity->GetType());
            if (isSpawner)
            {
                boost::shared_ptr<MobSpawnerBlockEntity> spawner = boost::static_pointer_cast<MobSpawnerBlockEntity>(
                    blockEntity);
                spawner->getSpawner()->setEntityId(EntityIO::getKey(EntityType::SILVERFISH));
            }
        }
    }
    return true;
}

StructurePiece* StrongholdPieces::PortalRoom::Create()
{
    return new PortalRoom();
}
EStructurePiece StrongholdPieces::PortalRoom::GetType()
{
    return eStructurePiece_PortalRoom;
}

void StrongholdPieces::FillerCorridor::addAdditionalSaveData(DataOutputStream* output)
{
    StrongholdPiece::addAdditionalSaveData(output);
    output->writeInt(mSteps);
}

void StrongholdPieces::FillerCorridor::readAdditionalSaveData(DataInputStream* input, StructureManager* manager)
{
    StrongholdPiece::readAdditionalSaveData(input, manager);
    mSteps = input->readInt();
}

bool StrongholdPieces::FillerCorridor::postProcess(Level* level, Random&, BoundingBox* box)
{
    if (edgesLiquid(level, box))
        return false;
    for (int z = 0; z < mSteps; ++z)
    {
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 0, 0, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 1, 0, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 2, 0, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 3, 0, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 4, 0, z, box);
        for (int y = 1; y <= 3; ++y)
        {
            placeBlock(level, Blocks::stonebrick->defaultBlockState(), 0, y, z, box);
            placeBlock(level, Blocks::air->defaultBlockState(), 1, y, z, box);
            placeBlock(level, Blocks::air->defaultBlockState(), 2, y, z, box);
            placeBlock(level, Blocks::air->defaultBlockState(), 3, y, z, box);
            placeBlock(level, Blocks::stonebrick->defaultBlockState(), 4, y, z, box);
        }
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 0, 4, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 1, 4, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 2, 4, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 3, 4, z, box);
        placeBlock(level, Blocks::stonebrick->defaultBlockState(), 4, 4, z, box);
    }
    return true;
}

StructurePiece* StrongholdPieces::FillerCorridor::Create()
{
    return new FillerCorridor();
}
EStructurePiece StrongholdPieces::FillerCorridor::GetType()
{
    return eStructurePiece_FillerCorridor;
}

StrongholdPieces::RightTurn::RightTurn() : LeftTurn()
{
}

StrongholdPieces::RightTurn::RightTurn(int depth, Random& random, BoundingBox* box, const Direction* direction)
    : LeftTurn(depth, random, box, direction)
{
}

void StrongholdPieces::RightTurn::addChildren(StructurePiece* start, std::vector<StructurePiece*>* pieces,
                                              Random& random)
{
    const Direction* orientation = getOrientation();
    if (orientation == Direction::NORTH || orientation == Direction::EAST)
        generateSmallDoorChildRight(static_cast<StartPiece*>(start), pieces, random, 1, 1);
    else
        generateSmallDoorChildLeft(static_cast<StartPiece*>(start), pieces, random, 1, 1);
}

bool StrongholdPieces::RightTurn::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (edgesLiquid(level, box))
        return false;
    generateBox(level, box, 0, 0, 0, 4, 4, 4, true, random, sStrongholdStoneSelector);
    generateSmallDoor(level, random, box, mEntryDoor, 1, 1, 0);
    const Direction* orientation = getOrientation();
    if (orientation == Direction::NORTH || orientation == Direction::EAST)
    {
        generateBox(level, box, 4, 1, 1, 4, 3, 3, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                    false);
    }
    else
    {
        generateBox(level, box, 0, 1, 1, 0, 3, 3, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                    false);
    }
    return true;
}

StructurePiece* StrongholdPieces::RightTurn::Create()
{
    return new RightTurn();
}
EStructurePiece StrongholdPieces::RightTurn::GetType()
{
    return eStructurePiece_RightTurn;
}

StrongholdPieces::StartPiece::StartPiece(int depth, Random& random, int x, int z, Level* level)
    : StairsDown(0, random, x, z)
{
    mLevel = level;
    mPortalRoom = NULL;
    mLastPieceWeight = NULL;
    mStartFlag = false;
}

StrongholdPieces::StartPiece::StartPiece()
    : StairsDown(),
      mStartFlag(false),
      mLastPieceWeight(NULL),
      mPortalRoom(NULL),
      mLevel(NULL),
      mPendingChildren()
{
}

BlockPos* StrongholdPieces::StartPiece::getLocatorPosition()
{
    if (mPortalRoom != NULL)
        return mPortalRoom->getLocatorPosition();
    return StructurePiece::getLocatorPosition();
}

EStructurePiece StrongholdPieces::StartPiece::GetType()
{
    return eStructurePiece_StrongholdStartPiece;
}

void StrongholdPieces::SmoothStoneSelector::next(Random& random, int, int, int, bool edge)
{
    if (!edge)
    {
        mNext = Blocks::air->defaultBlockState();
        return;
    }

    float roll = random.nextFloat();
    if (roll < 0.2f)
        mNext = Blocks::stonebrick->getBlockState(StoneBrickBlock::CRACKED_META);
    else if (roll < 0.5f)
        mNext = Blocks::stonebrick->getBlockState(StoneBrickBlock::MOSSY_META);
    else if (roll < 0.55f)
        mNext = Blocks::monster_egg->getBlockState(MonsterEggBlock::Variant::STONEBRICK->getData());
    else
        mNext = Blocks::stonebrick->defaultBlockState();
}

#include "game/world/generation/VillagePieces.hpp"

#include <cstdlib>

#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/Mth.hpp"
#include "game/util/Math.hpp"
#include "game/world/block/Material.hpp"

#include "game/world/biome/Biome.hpp"
#include "game/world/block/CropBlock.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DoorBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/LadderBlock.hpp"
#include "game/world/block/LogBlock.hpp"
#include "game/world/block/NewLogBlock.hpp"
#include "game/world/block/OldLogBlock.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/SandStoneBlock.hpp"
#include "game/world/block/StairBlock.hpp"
#include "game/world/block/TorchBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/StructureFeatureIO.hpp"
#include "game/world/generation/VillageFeature.hpp"
#include "game/world/generation/Villager.hpp"
#include "game/world/generation/ZombieVillager.hpp"

std::list<VillagePieces::PieceWeight*>* VillagePieces::createPieceSet(Random& random, int size)
{
    std::list<PieceWeight*>* pieces = new std::list<PieceWeight*>;
    pieces->push_back(new PieceWeight(ePieceClass_SimpleHouse, 4, Mth::nextInt(&random, size + 2, size * 2 + 4)));
    pieces->push_back(new PieceWeight(ePieceClass_SmallTemple, 20, Mth::nextInt(&random, size, size + 1)));
    pieces->push_back(new PieceWeight(ePieceClass_BookHouse, 20, Mth::nextInt(&random, size, size + 2)));
    pieces->push_back(new PieceWeight(ePieceClass_SmallHut, 3, Mth::nextInt(&random, size + 2, size * 3 + 5)));
    pieces->push_back(new PieceWeight(ePieceClass_PigHouse, 15, Mth::nextInt(&random, size, size + 2)));
    pieces->push_back(new PieceWeight(ePieceClass_DoubleFarmland, 3, Mth::nextInt(&random, size + 1, size + 4)));
    pieces->push_back(new PieceWeight(ePieceClass_Farmland, 3, Mth::nextInt(&random, size + 2, size * 2 + 4)));
    pieces->push_back(new PieceWeight(ePieceClass_Smithy, 15, Mth::nextInt(&random, 0, size + 1)));
    pieces->push_back(new PieceWeight(ePieceClass_TwoRoomHouse, 8, Mth::nextInt(&random, size, size * 2 + 3)));
    for (std::list<PieceWeight*>::iterator it = pieces->begin(); it != pieces->end();)
    {
        if ((*it)->maxPlaceCount == 0)
        {
            delete *it;
            it = pieces->erase(it);
        }
        else
        {
            ++it;
        }
    }
    return pieces;
}

void VillagePieces::loadStatic()
{
    StructureFeatureIO::setPieceId(eStructurePiece_BookHouse, BookHouse::Create, L"ViBH");
    StructureFeatureIO::setPieceId(eStructurePiece_DoubleFarmland, DoubleFarmland::Create, L"ViDF");
    StructureFeatureIO::setPieceId(eStructurePiece_Farmland, Farmland::Create, L"ViF");
    StructureFeatureIO::setPieceId(eStructurePiece_LightPost, LightPost::Create, L"ViL");
    StructureFeatureIO::setPieceId(eStructurePiece_PigHouse, PigHouse::Create, L"ViPH");
    StructureFeatureIO::setPieceId(eStructurePiece_SimpleHouse, SimpleHouse::Create, L"ViSH");
    StructureFeatureIO::setPieceId(eStructurePiece_SmallHut, SmallHut::Create, L"ViSmH");
    StructureFeatureIO::setPieceId(eStructurePiece_SmallTemple, SmallTemple::Create, L"ViST");
    StructureFeatureIO::setPieceId(eStructurePiece_Smithy, Smithy::Create, L"ViS");
    StructureFeatureIO::setPieceId(eStructurePiece_VillageStartPiece, StartPiece::Create, L"ViStart");
    StructureFeatureIO::setPieceId(eStructurePiece_StraightRoad, StraightRoad::Create, L"ViSR");
    StructureFeatureIO::setPieceId(eStructurePiece_TwoRoomHouse, TwoRoomHouse::Create, L"ViTRH");
    StructureFeatureIO::setPieceId(eStructurePiece_Well, Well::Create, L"ViW");
}

int VillagePieces::updatePieceWeight(std::list<PieceWeight*>* pieces)
{
    bool available = false;
    int total = 0;
    for (std::list<PieceWeight*>::iterator it = pieces->begin(); it != pieces->end(); ++it)
    {
        PieceWeight* piece = *it;
        if (piece->maxPlaceCount > 0 && piece->placeCount < piece->maxPlaceCount)
            available = true;
        total += piece->weight;
    }
    return available ? total : -1;
}

StructurePiece* VillagePieces::findAndCreatePieceFactory(StartPiece* startPiece, PieceWeight* weight,
                                                         std::vector<StructurePiece*>* pieces, Random& random, int x,
                                                         int y, int z, const Direction* direction, int depth)
{
    switch (weight->pieceClass)
    {
    case ePieceClass_SimpleHouse:
        return SimpleHouse::createPiece(startPiece, pieces, random, x, y, z, direction, depth);
    case ePieceClass_SmallTemple:
        return SmallTemple::createPiece(startPiece, pieces, random, x, y, z, direction, depth);
    case ePieceClass_BookHouse:
        return BookHouse::createPiece(startPiece, pieces, random, x, y, z, direction, depth);
    case ePieceClass_SmallHut:
        return SmallHut::createPiece(startPiece, pieces, random, x, y, z, direction, depth);
    case ePieceClass_PigHouse:
        return PigHouse::createPiece(startPiece, pieces, random, x, y, z, direction, depth);
    case ePieceClass_Smithy:
        return Smithy::createPiece(startPiece, pieces, random, x, y, z, direction, depth);
    case ePieceClass_DoubleFarmland:
        return DoubleFarmland::createPiece(startPiece, pieces, random, x, y, z, direction, depth);
    case ePieceClass_Farmland:
        return Farmland::createPiece(startPiece, pieces, random, x, y, z, direction, depth);
    case ePieceClass_TwoRoomHouse:
        return TwoRoomHouse::createPiece(startPiece, pieces, random, x, y, z, direction, depth);
    default:
        return NULL;
    }
}

StructurePiece* VillagePieces::generatePieceFromSmallDoor(StartPiece* startPiece, std::vector<StructurePiece*>* pieces,
                                                          Random& random, int x, int y, int z,
                                                          const Direction* direction, int depth)
{
    int total = updatePieceWeight(startPiece->mPieceWeights);
    if (total < 1)
        return NULL;

    int attempt = 0;
    while (attempt < 5)
    {
        ++attempt;
        int choice = random.nextInt(total);
        for (std::list<PieceWeight*>::iterator it = startPiece->mPieceWeights->begin();
             it != startPiece->mPieceWeights->end(); ++it)
        {
            PieceWeight* weight = *it;
            choice -= weight->weight;
            if (choice < 0)
            {
                if (!weight->doPlace(depth)
                    || (weight == startPiece->mLastPieceWeight && startPiece->mPieceWeights->size() > 1))
                    break;

                StructurePiece* piece = findAndCreatePieceFactory(startPiece, weight, pieces, random, x, y, z,
                                                                  direction, depth);
                if (piece != NULL)
                {
                    ++weight->placeCount;
                    startPiece->mLastPieceWeight = weight;
                    if (!weight->isValid())
                        startPiece->mPieceWeights->remove(weight);
                    return piece;
                }
            }
        }
    }

    BoundingBox* box = LightPost::findPieceBox(startPiece, pieces, random, x, y, z, direction);
    if (box != NULL)
        return new LightPost(startPiece, depth, random, box, direction);
    return NULL;
}

StructurePiece* VillagePieces::generateAndAddPiece(StartPiece* startPiece, std::vector<StructurePiece*>* pieces,
                                                   Random& random, int x, int y, int z, const Direction* direction,
                                                   int depth)
{
    if (depth > 50)
        return NULL;
    if (std::abs(x - startPiece->getBoundingBox()->x0) > 112 || std::abs(z - startPiece->getBoundingBox()->z0) > 112)
        return NULL;

    StructurePiece* piece = generatePieceFromSmallDoor(startPiece, pieces, random, x, y, z, direction, depth + 1);
    if (piece != NULL)
    {
        BoundingBox* pieceBox = piece->getBoundingBox();
        int centerX = (pieceBox->x0 + pieceBox->x1) / 2;
        int centerZ = (pieceBox->z0 + pieceBox->z1) / 2;
        int width = pieceBox->x1 - pieceBox->x0;
        int length = pieceBox->z1 - pieceBox->z0;
        int radius = width > length ? width : length;
        if (startPiece->getBiomeSource()->containsOnly(centerX, centerZ, radius / 2 + 4, VillageFeature::allowedBiomes))
        {
            pieces->push_back(piece);
            startPiece->mPendingPieces.push_back(piece);
            return piece;
        }
        delete piece;
    }
    return NULL;
}

StructurePiece* VillagePieces::generateAndAddRoadPiece(StartPiece* startPiece, std::vector<StructurePiece*>* pieces,
                                                       Random& random, int x, int y, int z, const Direction* direction,
                                                       int depth)
{
    if (depth > startPiece->mSize + 3)
        return NULL;
    if (std::abs(x - startPiece->getBoundingBox()->x0) > 112 || std::abs(z - startPiece->getBoundingBox()->z0) > 112)
        return NULL;

    BoundingBox* box = StraightRoad::findPieceBox(startPiece, pieces, random, x, y, z, direction);
    if (box == NULL)
        return NULL;
    if (box->y0 > 10)
    {
        StructurePiece* road = new StraightRoad(startPiece, depth, random, box, direction);
        pieces->push_back(road);
        startPiece->mRoadPieces.push_back(road);
        return road;
    }
    delete box;
    return NULL;
}

VillagePieces::PieceWeight::PieceWeight(EPieceClass kind, int chance, int maximum)
    : pieceClass(kind),
      weight(chance),
      placeCount(0),
      maxPlaceCount(maximum)
{
}

bool VillagePieces::PieceWeight::doPlace(int depth)
{
    return maxPlaceCount == 0 || placeCount < maxPlaceCount;
}

bool VillagePieces::PieceWeight::isValid()
{
    return maxPlaceCount == 0 || placeCount < maxPlaceCount;
}

VillagePieces::VillagePiece::VillagePiece(StartPiece* startPiece, int depth)
    : StructurePiece(depth),
      mFirstBools(),
      mSecondBools(),
      mFirstInts(),
      mSecondInts()
{
    mStartPiece = startPiece;
    mVillagerCount = 0;
    mAverageGroundHeight = -1;
    mIsZombieVillage = false;
    mType = 0;
    if (startPiece != NULL)
    {
        mType = startPiece->mType;
        mIsZombieVillage = startPiece->mIsZombieVillage;
    }
    mCobblestoneSelector = new CobbleStoneSelector(mIsZombieVillage, mType);
    mSecondInts.clear();
}

VillagePieces::VillagePiece::VillagePiece()
    : StructurePiece(),
      mFirstBools(),
      mSecondBools(),
      mFirstInts(),
      mSecondInts()
{
    mStartPiece = NULL;
    mAverageGroundHeight = -1;
    mVillagerCount = 0;
    mType = 0;
    mIsZombieVillage = false;
    mSecondBools.clear();
    mFirstInts.clear();
    mFirstBools.clear();
    mSecondInts.clear();
    mCobblestoneSelector = new CobbleStoneSelector(mIsZombieVillage, mType);
}

VillagePieces::VillagePiece::~VillagePiece()
{
    delete mCobblestoneSelector;
    mCobblestoneSelector = NULL;
}

void VillagePieces::VillagePiece::setStructureType(int type)
{
    mType = type;
}

bool VillagePieces::VillagePiece::isOkBox(BoundingBox* box, StartPiece* startPiece)
{
    bool ok = false;
    if (box != NULL)
    {
        if (box->y0 > 10)
            ok = true;
        int xzSize = startPiece->mLevel->getLevelData()->getXZSize();
        int lower = -((xzSize << 4) / 2) + 1;
        int upper = ((xzSize << 4) / 2) - 1;
        if (box->x0 <= lower)
            ok = false;
        if (box->z0 <= lower)
            ok = false;
        if (box->x1 >= upper)
            ok = false;
        if (box->z1 >= upper)
            ok = false;
    }
    return ok;
}

void VillagePieces::VillagePiece::addAdditionalSaveData(DataOutputStream* output)
{
    if (mStartPiece != NULL)
        mType = mStartPiece->mType;
    output->writeInt(mAverageGroundHeight);
    output->writeInt(mVillagerCount);
    output->writeByte(static_cast<unsigned char>(mType));
    output->writeBoolean(mIsZombieVillage);
}

void VillagePieces::VillagePiece::readAdditionalSaveData(DataInputStream* input, StructureManager* manager)
{
    mAverageGroundHeight = input->readInt();
    if (StructureFeatureIO::getVersion() == 1)
        readStructurePieceSaveData(input, mFirstBools, mSecondBools, mFirstInts, mSecondInts);
    mVillagerCount = input->readInt();
    if (StructureFeatureIO::getVersion() < 2)
    {
        if (input->readBoolean())
            mType = 1;
    }
    else
    {
        mType = input->readByte();
        mIsZombieVillage = input->readBoolean();
        delete mCobblestoneSelector;
        mCobblestoneSelector = new CobbleStoneSelector(mIsZombieVillage, mType);
    }
    if (mStartPiece != NULL)
        mType = mStartPiece->mType;
}

StructurePiece* VillagePieces::VillagePiece::generateHouseNorthernLeft(StartPiece* startPiece,
                                                                       std::vector<StructurePiece*>* pieces,
                                                                       Random& random, int x, int z)
{
    const Direction* orientation = getOrientation();
    if (orientation == NULL)
        return NULL;
    if (orientation->get3DDataValue() > 3 && orientation->get3DDataValue() < 6)
        return generateAndAddPiece(startPiece, pieces, random, getBoundingBox()->x0 + z, getBoundingBox()->y0 + x,
                                   getBoundingBox()->z0 - 1, Direction::NORTH, getGenDepth());
    return generateAndAddPiece(startPiece, pieces, random, getBoundingBox()->x0 - 1, getBoundingBox()->y0 + x,
                               getBoundingBox()->z0 + z, Direction::WEST, getGenDepth());
}

StructurePiece* VillagePieces::VillagePiece::generateHouseNorthernRight(StartPiece* startPiece,
                                                                        std::vector<StructurePiece*>* pieces,
                                                                        Random& random, int x, int z)
{
    const Direction* orientation = getOrientation();
    if (orientation == NULL)
        return NULL;
    if (orientation->get3DDataValue() > 3 && orientation->get3DDataValue() < 6)
        return generateAndAddPiece(startPiece, pieces, random, getBoundingBox()->x0 + z, getBoundingBox()->y0 + x,
                                   getBoundingBox()->z1 + 1, Direction::SOUTH, getGenDepth());
    return generateAndAddPiece(startPiece, pieces, random, getBoundingBox()->x1 + 1, getBoundingBox()->y0 + x,
                               getBoundingBox()->z0 + z, Direction::EAST, getGenDepth());
}

int VillagePieces::VillagePiece::getAverageGroundHeight(Level* level, BoundingBox* box)
{
    int total = 0;
    int count = 0;
    BlockPos pos;
    for (int z = mBoundingBox->z0; z <= mBoundingBox->z1; ++z)
    {
        for (int x = mBoundingBox->x0; x <= mBoundingBox->x1; ++x)
        {
            pos = BlockPos(x, 64, z);
            if (box->isInside(pos))
            {
                int height = Math::_max(level->getTopSolidBlockPos(pos).y, level->mDimension->getSpawnYPosition() - 1);
                total += height;
                ++count;
            }
        }
    }
    if (count == 0)
        return -1;
    return total / count;
}

void VillagePieces::VillagePiece::spawnVillagers(Level* level, BoundingBox* box, int x, int y, int z, int count)
{
    if (mVillagerCount < count)
    {
        for (int i = mVillagerCount; i < count; ++i)
        {
            int worldX = getWorldX(x + i, z);
            int worldY = getWorldY(y);
            int worldZ = getWorldZ(x + i, z);
            if (!box->isInside(BlockPos(worldX, worldY, worldZ)))
                break;

            ++mVillagerCount;
            if (mIsZombieVillage)
            {
                boost::shared_ptr<ZombieVillager> zombie(new ZombieVillager(level));
                zombie->moveTo(worldX + 0.5, worldY, worldZ + 0.5, 0.0f, 0.0f);
                zombie->finalizeMobSpawn(level->getCurrentDifficultyAt(BlockPos(zombie)), NULL, L"");
                zombie->setProfession(getVillagerProfession(i, 0));
                zombie->setPersistenceRequired(true);
                level->addEntity(zombie);
            }
            else
            {
                boost::shared_ptr<Villager> villager(new Villager(level));
                villager->moveTo(worldX + 0.5, worldY, worldZ + 0.5, 0.0f, 0.0f);
                villager->setProfession(getVillagerProfession(i, level->mRandom->nextInt(6)));
                villager->finalizeMobSpawn(level->getCurrentDifficultyAt(BlockPos(villager)), NULL, L"");
                villager->setPersistenceRequired(true);
                level->addEntity(villager);
            }
        }
    }
}

int VillagePieces::VillagePiece::getVillagerProfession(int x, int z)
{
    return z;
}

const BlockState* VillagePieces::VillagePiece::biomeBlock(const BlockState* state)
{
    if (mType == 1)
    {
        if (state->getBlock() == Blocks::log || state->getBlock() == Blocks::log2)
            return Blocks::sandstone->defaultBlockState();
        if (state->getBlock() == Blocks::cobblestone)
            return Blocks::sandstone->getBlockState(SandStoneBlock::SANDSTONE->getData());
        if (state->getBlock() == Blocks::planks)
            return Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData());
        if (state->getBlock() == Blocks::oak_stairs)
        {
            return Blocks::sandstone_stairs->defaultBlockState()->setValue(StairBlock::FACING,
                                                                           state->getValue(StairBlock::FACING));
        }
        if (state->getBlock() == Blocks::stone_stairs)
        {
            return Blocks::sandstone_stairs->defaultBlockState()->setValue(StairBlock::FACING,
                                                                           state->getValue(StairBlock::FACING));
        }
        if (state->getBlock() == Blocks::grass_path)
            return Blocks::sandstone->defaultBlockState();
    }
    else if (mType == 3)
    {
        if (state->getBlock() == Blocks::log || state->getBlock() == Blocks::log2)
        {
            const BlockState* logState = Blocks::log->defaultBlockState();
            logState = logState->setValue(OldLogBlock::VARIANT, PlanksBlock::Variant::SPRUCE);
            EnumProperty<LogBlock::LogAxis*>* axis = LogBlock::AXIS;
            return logState->setValue(axis, state->getValue(axis));
        }
        if (state->getBlock() == Blocks::planks)
            return Blocks::planks->defaultBlockState()->setValue(PlanksBlock::VARIANT, PlanksBlock::Variant::SPRUCE);
        if (state->getBlock() == Blocks::oak_stairs)
        {
            const BlockState* stairs = Blocks::spruce_stairs->defaultBlockState();
            DirectionProperty* facing = StairBlock::FACING;
            return stairs->setValue(facing, state->getValue(facing));
        }
        if (state->getBlock() == Blocks::fence)
            return Blocks::spruce_fence->defaultBlockState();
    }
    else if (mType == 2)
    {
        if (state->getBlock() == Blocks::log || state->getBlock() == Blocks::log2)
        {
            const BlockState* logState = Blocks::log2->defaultBlockState();
            logState = logState->setValue(NewLogBlock::VARIANT, PlanksBlock::Variant::ACACIA);
            EnumProperty<LogBlock::LogAxis*>* axis = LogBlock::AXIS;
            return logState->setValue(axis, state->getValue(axis));
        }
        if (state->getBlock() == Blocks::planks)
            return Blocks::planks->defaultBlockState()->setValue(PlanksBlock::VARIANT, PlanksBlock::Variant::ACACIA);
        if (state->getBlock() == Blocks::oak_stairs)
        {
            const BlockState* stairs = Blocks::acacia_stairs->defaultBlockState();
            DirectionProperty* facing = StairBlock::FACING;
            return stairs->setValue(facing, state->getValue(facing));
        }
        if (state->getBlock() == Blocks::fence)
            return Blocks::acacia_fence->defaultBlockState();
    }
    return state;
}

DoorBlock* VillagePieces::VillagePiece::biomeDoor()
{
    switch (mType)
    {
    case 2:
        return static_cast<DoorBlock*>(Blocks::acacia_door);
    case 3:
        return static_cast<DoorBlock*>(Blocks::spruce_door);
    default:
        return static_cast<DoorBlock*>(Blocks::wooden_door);
    }
}

void VillagePieces::VillagePiece::createVillageDoor(Level* level, BoundingBox* box, Random& random, int x, int y, int z,
                                                    const Direction* direction)
{
    if (!mIsZombieVillage)
    {
        createDoor(level, box, random, x, y, z, Direction::NORTH, biomeDoor());
    }
}

void VillagePieces::VillagePiece::placeTorch(Level* level, const Direction* direction, int x, int y, int z,
                                             BoundingBox* box)
{
    if (!mIsZombieVillage)
    {
        placeBlock(level, Blocks::torch->defaultBlockState()->setValue(TorchBlock::FACING, direction), x, y, z, box);
    }
}

void VillagePieces::VillagePiece::placeBlock(Level* level, Random& random, const BlockState* state,
                                             StructurePiece::BlockSelector* selector, int x, int y, int z,
                                             BoundingBox* box)
{
    if (selector != NULL)
    {
        selector->next(random, x, y, z, false);
        StructurePiece::placeBlock(level, selector->getNext(), x, y, z, box);
        return;
    }
    StructurePiece::placeBlock(level, state, x, y, z, box);
}

VillagePieces::Well::Well(StartPiece* startPiece, int depth, Random& random, int x, int z)
    : VillagePiece(startPiece, depth)
{
    setOrientation(Direction::Plane::HORIZONTAL->getRandomDirection(random));
    if (getOrientation()->getAxis() == Direction::Axis::Z)
        mBoundingBox = new BoundingBox(x, 64, z, x + 5, 78, z + 5);
    else
        mBoundingBox = new BoundingBox(x, 64, z, x + 5, 78, z + 5);
}

VillagePieces::Well::Well() : VillagePiece()
{
}

void VillagePieces::Well::addChildren(StructurePiece* startPiece, std::vector<StructurePiece*>* pieces, Random& random)
{
    StartPiece* start = static_cast<StartPiece*>(startPiece);
    generateAndAddRoadPiece(start, pieces, random, mBoundingBox->x0 - 1, mBoundingBox->y1 - 4, mBoundingBox->z0 + 1,
                            Direction::WEST, getGenDepth());
    generateAndAddRoadPiece(start, pieces, random, mBoundingBox->x1 + 1, mBoundingBox->y1 - 4, mBoundingBox->z0 + 1,
                            Direction::EAST, getGenDepth());
    generateAndAddRoadPiece(start, pieces, random, mBoundingBox->x0 + 1, mBoundingBox->y1 - 4, mBoundingBox->z0 - 1,
                            Direction::NORTH, getGenDepth());
    generateAndAddRoadPiece(start, pieces, random, mBoundingBox->x0 + 1, mBoundingBox->y1 - 4, mBoundingBox->z1 + 1,
                            Direction::SOUTH, getGenDepth());
}

bool VillagePieces::Well::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (mAverageGroundHeight < 0)
    {
        mAverageGroundHeight = getAverageGroundHeight(level, box);
        if (mAverageGroundHeight < 0)
            return true;
        mBoundingBox->move(0, mAverageGroundHeight - mBoundingBox->y1 + 3, 0);
    }

    const BlockState* path = biomeBlock(Blocks::grass_path->defaultBlockState());
    const BlockState* cobblestone = biomeBlock(Blocks::cobblestone->defaultBlockState());
    const BlockState* fence = biomeBlock(Blocks::fence->defaultBlockState());
    Random selectorRandom = random;

    generateBox(level, box, 1, 0, 1, 4, 12, 4, cobblestone, Blocks::flowing_water->defaultBlockState(), false);
    placeBlock(level, Blocks::air->defaultBlockState(), 2, 12, 2, box);
    placeBlock(level, Blocks::air->defaultBlockState(), 3, 12, 2, box);
    placeBlock(level, Blocks::air->defaultBlockState(), 2, 12, 3, box);
    placeBlock(level, Blocks::air->defaultBlockState(), 3, 12, 3, box);
    placeBlock(level, fence, 1, 13, 1, box);
    placeBlock(level, fence, 1, 14, 1, box);
    placeBlock(level, fence, 4, 13, 1, box);
    placeBlock(level, fence, 4, 14, 1, box);
    placeBlock(level, fence, 1, 13, 4, box);
    placeBlock(level, fence, 1, 14, 4, box);
    placeBlock(level, fence, 4, 13, 4, box);
    placeBlock(level, fence, 4, 14, 4, box);
    generateBox(level, box, 1, 15, 1, 4, 15, 4, false, selectorRandom, mCobblestoneSelector);

    for (int z = 0; z <= 5; ++z)
    {
        for (int x = 0; x <= 5; ++x)
        {
            if (x == 0 || x == 5 || z == 0 || z == 5)
            {
                placeBlock(level, path, x, 11, z, box);
                generateAirColumnUp(level, x, 12, z, box);
            }
        }
    }
    return true;
}

StructurePiece* VillagePieces::Well::Create()
{
    return new Well();
}

EStructurePiece VillagePieces::Well::GetType()
{
    return eStructurePiece_Well;
}

VillagePieces::StartPiece::StartPiece(BiomeSource* source, int depth, Random& random, int x, int z,
                                      std::list<PieceWeight*>* pieces, int size, Level* level)
    : Well(this, 0, random, x, z),
      mBiomeSource(source),
      mSize(size),
      mHasPlacedWell(false),
      mLastPieceWeight(NULL),
      mPieceWeights(pieces),
      mLevel(level),
      mPendingPieces(),
      mRoadPieces()
{
    Biome* biome = source->getBiome(BlockPos(x, 0, z), Biome::DEFAULT);
    if (biome == Biome::DESERT || biome == Biome::DESERT_HILLS)
    {
        mType = 1;
    }
    else if (biome == Biome::SAVANNA || biome == Biome::SAVANNA_MUTATED || biome == Biome::SAVANNA_PLATEAU
             || biome == Biome::SAVANNA_PLATEAU_MUTATED)
    {
        mType = 2;
    }
    else if (biome == Biome::ICE_PLAINS || biome == Biome::TAIGA || biome == Biome::COLD_TAIGA
             || biome == Biome::COLD_TAIGA_HILLS || biome == Biome::COLD_TAIGA_MUTATED)
    {
        mType = 3;
    }
    VillagePiece::setStructureType(mType);
    mIsZombieVillage = random.nextInt(50) == 0;
    delete mCobblestoneSelector;
    mCobblestoneSelector = new CobbleStoneSelector(mIsZombieVillage, mType);
}

VillagePieces::StartPiece::~StartPiece()
{
    for (std::list<PieceWeight*>::iterator it = mPieceWeights->begin(); it != mPieceWeights->end(); ++it)
    {
        delete *it;
    }
    delete mPieceWeights;
}

BiomeSource* VillagePieces::StartPiece::getBiomeSource()
{
    return mBiomeSource;
}

EStructurePiece VillagePieces::StartPiece::GetType()
{
    return eStructurePiece_VillageStartPiece;
}

VillagePieces::StraightRoad::StraightRoad() : VillageRoadPiece()
{
}

VillagePieces::StraightRoad::StraightRoad(StartPiece* startPiece, int depth, Random& random, BoundingBox* box,
                                          const Direction* direction)
    : VillageRoadPiece(startPiece, depth)
{
    setOrientation(direction);
    mBoundingBox = box;
    mLength = Math::_max(box->getXSpan(), box->getZSpan());
}

bool VillagePieces::StraightRoad::postProcess(Level* level, Random& random, BoundingBox* box)
{
    const BlockState* path = biomeBlock(Blocks::grass_path->defaultBlockState());
    const BlockState* planks = biomeBlock(Blocks::planks->defaultBlockState());
    const BlockState* sandPath = biomeBlock(Blocks::grass_path->defaultBlockState());
    const BlockState* cobblestone = biomeBlock(Blocks::cobblestone->defaultBlockState());

    for (int x = mBoundingBox->x0; x <= mBoundingBox->x1; ++x)
    {
        for (int z = mBoundingBox->z0; z <= mBoundingBox->z1; ++z)
        {
            BlockPos pos(x, 64, z);
            if (!box->isInside(pos))
                continue;

            pos = level->getTopSolidBlockPos(pos).below();
            if (pos.y < level->getSeaLevel())
                pos = BlockPos(pos.x, level->getSeaLevel() - 1, pos.z);

            while (pos.y >= level->getSeaLevel() - 1)
            {
                const BlockState* state = level->getBlockState(pos);
                if (state->getBlock() == Blocks::grass && level->isEmptyBlock(pos.above()))
                {
                    level->setBlock(pos, path, Block::UPDATE_CLIENTS, false);
                    break;
                }
                if (state->getBlock()->mMaterial->isLiquid())
                {
                    level->setBlock(pos, planks, Block::UPDATE_CLIENTS, false);
                    break;
                }
                if (state->getBlock() == Blocks::sand || state->getBlock() == Blocks::sandstone
                    || state->getBlock() == Blocks::red_sandstone)
                {
                    level->setBlock(pos, sandPath, Block::UPDATE_CLIENTS, false);
                    level->setBlock(pos.below(), cobblestone, Block::UPDATE_CLIENTS, false);
                    break;
                }
                pos = pos.below();
            }
        }
    }
    return true;
}

StructurePiece* VillagePieces::StraightRoad::Create()
{
    return new StraightRoad();
}

EStructurePiece VillagePieces::StraightRoad::GetType()
{
    return eStructurePiece_StraightRoad;
}

void VillagePieces::StraightRoad::addAdditionalSaveData(DataOutputStream* output)
{
    VillagePiece::addAdditionalSaveData(output);
    output->writeInt(mLength);
}

void VillagePieces::StraightRoad::readAdditionalSaveData(DataInputStream* input, StructureManager* manager)
{
    VillagePiece::readAdditionalSaveData(input, manager);
    mLength = input->readInt();
}

void VillagePieces::StraightRoad::addChildren(StructurePiece* startPiece, std::vector<StructurePiece*>* pieces,
                                              Random& random)
{
    StartPiece* start = static_cast<StartPiece*>(startPiece);
    bool placedHouse = false;
    for (int offset = random.nextInt(5); offset < mLength - 8; offset += 2 + random.nextInt(5))
    {
        StructurePiece* house = generateHouseNorthernLeft(start, pieces, random, 0, offset);
        if (house != NULL)
        {
            offset += Math::_max(house->getBoundingBox()->getXSpan(), house->getBoundingBox()->getZSpan());
            placedHouse = true;
        }
    }
    for (int offset = random.nextInt(5); offset < mLength - 8; offset += 2 + random.nextInt(5))
    {
        StructurePiece* house = generateHouseNorthernRight(start, pieces, random, 0, offset);
        if (house != NULL)
        {
            offset += Math::_max(house->getBoundingBox()->getXSpan(), house->getBoundingBox()->getZSpan());
            placedHouse = true;
        }
    }

    const Direction* orientation = getOrientation();
    if (!placedHouse)
        return;

    if (random.nextInt(3) > 0 && orientation != NULL)
    {
        switch (orientation->get3DDataValue())
        {
        case 3:
            generateAndAddRoadPiece(start, pieces, random, mBoundingBox->x0 - 1, mBoundingBox->y0, mBoundingBox->z1 - 2,
                                    Direction::WEST, getGenDepth());
            break;
        case 4:
            generateAndAddRoadPiece(start, pieces, random, mBoundingBox->x0, mBoundingBox->y0, mBoundingBox->z0 - 1,
                                    Direction::NORTH, getGenDepth());
            break;
        case 5:
            generateAndAddRoadPiece(start, pieces, random, mBoundingBox->x1 - 2, mBoundingBox->y0, mBoundingBox->z0 - 1,
                                    Direction::NORTH, getGenDepth());
            break;
        default:
            generateAndAddRoadPiece(start, pieces, random, mBoundingBox->x0 - 1, mBoundingBox->y0, mBoundingBox->z0,
                                    Direction::WEST, getGenDepth());
            break;
        }
    }

    if (random.nextInt(3) > 0 && orientation != NULL)
    {
        switch (orientation->get3DDataValue())
        {
        case 3:
            generateAndAddRoadPiece(start, pieces, random, mBoundingBox->x1 + 1, mBoundingBox->y0, mBoundingBox->z1 - 2,
                                    Direction::EAST, getGenDepth());
            break;
        case 4:
            generateAndAddRoadPiece(start, pieces, random, mBoundingBox->x0, mBoundingBox->y0, mBoundingBox->z1 + 1,
                                    Direction::SOUTH, getGenDepth());
            break;
        case 5:
            generateAndAddRoadPiece(start, pieces, random, mBoundingBox->x1 - 2, mBoundingBox->y0, mBoundingBox->z1 + 1,
                                    Direction::SOUTH, getGenDepth());
            break;
        default:
            generateAndAddRoadPiece(start, pieces, random, mBoundingBox->x1 + 1, mBoundingBox->y0, mBoundingBox->z0,
                                    Direction::EAST, getGenDepth());
            break;
        }
    }
}

BoundingBox* VillagePieces::StraightRoad::findPieceBox(StartPiece* startPiece, std::vector<StructurePiece*>* pieces,
                                                       Random& random, int x, int y, int z, const Direction* direction)
{
    for (int length = Mth::nextInt(&random, 3, 5) * 7; length >= 7; length -= 7)
    {
        BoundingBox* box = BoundingBox::orientBox(x, y, z, 0, 0, 0, 3, 3, length, direction);
        if (isOkBox(box, startPiece) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
            return box;
        delete box;
    }
    return NULL;
}

VillagePieces::SimpleHouse::SimpleHouse() : VillagePiece(), mIsRoofAccessible(false)
{
}

VillagePieces::SimpleHouse::SimpleHouse(StartPiece* startPiece, int depth, Random& random, BoundingBox* box,
                                        const Direction* direction)
    : VillagePiece(startPiece, depth)
{
    mIsRoofAccessible = random.nextBoolean();
    setOrientation(direction);
    mBoundingBox = box;
}

StructurePiece* VillagePieces::SimpleHouse::createPiece(StartPiece* startPiece, std::vector<StructurePiece*>* pieces,
                                                        Random& random, int x, int y, int z, const Direction* direction,
                                                        int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, 0, 0, 0, 5, 6, 5, direction);
    if (VillagePiece::isOkBox(box, startPiece) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new SimpleHouse(startPiece, depth, random, box, direction);
    delete box;
    return NULL;
}

bool VillagePieces::SimpleHouse::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (mAverageGroundHeight < 0)
    {
        mAverageGroundHeight = getAverageGroundHeight(level, box);
        if (mAverageGroundHeight < 0)
            return true;
        mBoundingBox->move(0, mAverageGroundHeight - mBoundingBox->y1 + 5, 0);
    }

    const BlockState* cobblestone = biomeBlock(Blocks::cobblestone->defaultBlockState());
    const BlockState* planks = biomeBlock(Blocks::planks->defaultBlockState());
    const BlockState* stairsNorth = biomeBlock(
        Blocks::stone_stairs->defaultBlockState()->setValue(StairBlock::FACING, Direction::NORTH));
    const BlockState* log = biomeBlock(Blocks::log->defaultBlockState());
    const BlockState* fence = biomeBlock(Blocks::fence->defaultBlockState());
    Random selectorRandom = random;

    generateBox(level, box, 0, 0, 0, 4, 0, 4, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 0, 4, 0, 4, 4, 4, log, log, false);
    generateBox(level, box, 1, 4, 1, 3, 4, 3, planks, planks, false);
    placeBlock(level, selectorRandom, cobblestone, mCobblestoneSelector, 0, 1, 0, box);
    placeBlock(level, selectorRandom, cobblestone, mCobblestoneSelector, 0, 2, 0, box);
    placeBlock(level, selectorRandom, cobblestone, mCobblestoneSelector, 0, 3, 0, box);
    placeBlock(level, selectorRandom, cobblestone, mCobblestoneSelector, 4, 1, 0, box);
    placeBlock(level, selectorRandom, cobblestone, mCobblestoneSelector, 4, 2, 0, box);
    placeBlock(level, selectorRandom, cobblestone, mCobblestoneSelector, 4, 3, 0, box);
    placeBlock(level, selectorRandom, cobblestone, mCobblestoneSelector, 0, 1, 4, box);
    placeBlock(level, selectorRandom, cobblestone, mCobblestoneSelector, 0, 2, 4, box);
    placeBlock(level, selectorRandom, cobblestone, mCobblestoneSelector, 0, 3, 4, box);
    placeBlock(level, selectorRandom, cobblestone, mCobblestoneSelector, 4, 1, 4, box);
    placeBlock(level, selectorRandom, cobblestone, mCobblestoneSelector, 4, 2, 4, box);
    placeBlock(level, selectorRandom, cobblestone, mCobblestoneSelector, 4, 3, 4, box);
    generateBox(level, box, 0, 1, 1, 0, 3, 3, planks, planks, false);
    generateBox(level, box, 4, 1, 1, 4, 3, 3, planks, planks, false);
    generateBox(level, box, 1, 1, 4, 3, 3, 4, planks, planks, false);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 0, 2, 2, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 2, 2, 4, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 4, 2, 2, box);
    placeBlock(level, planks, 1, 1, 0, box);
    placeBlock(level, planks, 1, 2, 0, box);
    placeBlock(level, planks, 1, 3, 0, box);
    placeBlock(level, planks, 2, 3, 0, box);
    placeBlock(level, planks, 3, 3, 0, box);
    placeBlock(level, planks, 3, 2, 0, box);
    placeBlock(level, planks, 3, 1, 0, box);
    if (getBlock(level, 2, 0, -1, box)->getBlock()->mMaterial == Material::AIR
        && getBlock(level, 2, -1, -1, box)->getBlock()->mMaterial != Material::AIR)
    {
        placeBlock(level, stairsNorth, 2, 0, -1, box);
        if (getBlock(level, 2, -1, -1, box)->getBlock() == Blocks::grass_path)
            placeBlock(level, Blocks::grass->defaultBlockState(), 2, -1, -1, box);
    }

    generateBox(level, box, 1, 1, 1, 3, 3, 3, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    if (mIsZombieVillage)
    {
        placeBlock(level, Blocks::web->defaultBlockState(), Mth::nextInt(&random, 1, 3), 3, 3, box);
        placeBlock(level, Blocks::web->defaultBlockState(), 1, 3, Mth::nextInt(&random, 1, 3), box);
    }

    if (mIsRoofAccessible)
    {
        placeBlock(level, fence, 0, 5, 0, box);
        placeBlock(level, fence, 1, 5, 0, box);
        placeBlock(level, fence, 2, 5, 0, box);
        placeBlock(level, fence, 3, 5, 0, box);
        placeBlock(level, fence, 4, 5, 0, box);
        placeBlock(level, fence, 0, 5, 4, box);
        placeBlock(level, fence, 1, 5, 4, box);
        placeBlock(level, fence, 2, 5, 4, box);
        placeBlock(level, fence, 3, 5, 4, box);
        placeBlock(level, fence, 4, 5, 4, box);
        placeBlock(level, fence, 4, 5, 1, box);
        placeBlock(level, fence, 4, 5, 2, box);
        placeBlock(level, fence, 4, 5, 3, box);
        placeBlock(level, fence, 0, 5, 1, box);
        placeBlock(level, fence, 0, 5, 2, box);
        placeBlock(level, fence, 0, 5, 3, box);
    }

    if (mIsRoofAccessible)
    {
        const BlockState* ladder = Blocks::ladder->defaultBlockState()->setValue(LadderBlock::FACING, Direction::SOUTH);
        placeBlock(level, ladder, 3, 1, 3, box);
        placeBlock(level, ladder, 3, 2, 3, box);
        placeBlock(level, ladder, 3, 3, 3, box);
        placeBlock(level, ladder, 3, 4, 3, box);
    }

    placeTorch(level, Direction::NORTH, 2, 3, 1, box);
    generateAirBoxUp(level, 0, 0, 5, 5, 6, box);
    fillBoxDown(level, cobblestone, 0, 0, 5, 5, -1, box);
    spawnVillagers(level, box, 1, 1, 2, 1);
    return true;
}

StructurePiece* VillagePieces::SimpleHouse::Create()
{
    return new SimpleHouse();
}

EStructurePiece VillagePieces::SimpleHouse::GetType()
{
    return eStructurePiece_SimpleHouse;
}

void VillagePieces::SimpleHouse::addAdditionalSaveData(DataOutputStream* output)
{
    VillagePiece::addAdditionalSaveData(output);
    output->writeBoolean(mIsRoofAccessible);
}

void VillagePieces::SimpleHouse::readAdditionalSaveData(DataInputStream* input, StructureManager* manager)
{
    VillagePiece::readAdditionalSaveData(input, manager);
    if (StructureFeatureIO::getVersion() == 1)
        mIsRoofAccessible = !mSecondBools.empty() ? mSecondBools[0] : false;
    else
        mIsRoofAccessible = input->readBoolean();
}

VillagePieces::SmallTemple::SmallTemple() : VillagePiece()
{
}

VillagePieces::SmallTemple::SmallTemple(StartPiece* startPiece, int depth, Random& random, BoundingBox* box,
                                        const Direction* direction)
    : VillagePiece(startPiece, depth)
{
    setOrientation(direction);
    mBoundingBox = box;
}

StructurePiece* VillagePieces::SmallTemple::createPiece(StartPiece* startPiece, std::vector<StructurePiece*>* pieces,
                                                        Random& random, int x, int y, int z, const Direction* direction,
                                                        int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, 0, 0, 0, 5, 12, 9, direction);
    if (isOkBox(box, startPiece) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new SmallTemple(startPiece, depth, random, box, direction);
    delete box;
    return NULL;
}

bool VillagePieces::SmallTemple::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (mAverageGroundHeight < 0)
    {
        mAverageGroundHeight = getAverageGroundHeight(level, box);
        if (mAverageGroundHeight < 0)
            return true;
        mBoundingBox->move(0, mAverageGroundHeight - mBoundingBox->y1 + 11, 0);
    }

    const BlockState* cobblestone = biomeBlock(Blocks::cobblestone->defaultBlockState());
    const BlockState* stairsNorth = biomeBlock(
        Blocks::stone_stairs->defaultBlockState()->setValue(StairBlock::FACING, Direction::NORTH));
    const BlockState* stairsWest = biomeBlock(
        Blocks::stone_stairs->defaultBlockState()->setValue(StairBlock::FACING, Direction::WEST));
    const BlockState* stairsEast = biomeBlock(
        Blocks::stone_stairs->defaultBlockState()->setValue(StairBlock::FACING, Direction::EAST));
    Random selectorRandom = random;

    generateBox(level, box, 1, 1, 1, 3, 3, 7, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 1, 5, 1, 3, 9, 3, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 1, 0, 0, 3, 0, 8, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 1, 1, 0, 3, 10, 0, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 0, 1, 1, 0, 10, 3, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 4, 1, 1, 4, 10, 3, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 0, 0, 4, 0, 4, 7, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 4, 0, 4, 4, 4, 7, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 1, 1, 8, 3, 4, 8, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 1, 5, 4, 3, 10, 4, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 1, 5, 5, 3, 5, 7, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 0, 9, 0, 4, 9, 4, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 0, 4, 0, 4, 4, 4, false, selectorRandom, mCobblestoneSelector);
    placeBlock(level, selectorRandom, cobblestone, mCobblestoneSelector, 0, 11, 2, box);
    placeBlock(level, selectorRandom, cobblestone, mCobblestoneSelector, 4, 11, 2, box);
    placeBlock(level, selectorRandom, cobblestone, mCobblestoneSelector, 2, 11, 0, box);
    placeBlock(level, selectorRandom, cobblestone, mCobblestoneSelector, 2, 11, 4, box);
    placeBlock(level, selectorRandom, cobblestone, mCobblestoneSelector, 1, 1, 6, box);
    placeBlock(level, selectorRandom, cobblestone, mCobblestoneSelector, 1, 1, 7, box);
    placeBlock(level, selectorRandom, cobblestone, mCobblestoneSelector, 2, 1, 7, box);
    placeBlock(level, selectorRandom, cobblestone, mCobblestoneSelector, 3, 1, 6, box);
    placeBlock(level, selectorRandom, cobblestone, mCobblestoneSelector, 3, 1, 7, box);
    placeBlock(level, stairsNorth, 1, 1, 5, box);
    placeBlock(level, stairsNorth, 2, 1, 6, box);
    placeBlock(level, stairsNorth, 3, 1, 5, box);
    placeBlock(level, stairsWest, 1, 2, 7, box);
    placeBlock(level, stairsEast, 3, 2, 7, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 0, 2, 2, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 0, 3, 2, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 4, 2, 2, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 4, 3, 2, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 0, 6, 2, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 0, 7, 2, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 4, 6, 2, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 4, 7, 2, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 2, 6, 0, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 2, 7, 0, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 2, 6, 4, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 2, 7, 4, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 0, 3, 6, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 4, 3, 6, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 2, 3, 8, box);
    placeTorch(level, Direction::SOUTH, 2, 4, 7, box);
    placeTorch(level, Direction::EAST, 1, 4, 6, box);
    placeTorch(level, Direction::WEST, 3, 4, 6, box);
    placeTorch(level, Direction::NORTH, 2, 4, 5, box);

    const BlockState* ladder = Blocks::ladder->defaultBlockState()->setValue(LadderBlock::FACING, Direction::WEST);
    for (int y = 1; y <= 9; ++y)
        placeBlock(level, ladder, 3, y, 3, box);

    placeBlock(level, Blocks::air->defaultBlockState(), 2, 1, 0, box);
    placeBlock(level, Blocks::air->defaultBlockState(), 2, 2, 0, box);
    createVillageDoor(level, box, random, 2, 1, 0, Direction::NORTH);
    if (getBlock(level, 2, 0, -1, box)->getBlock()->mMaterial == Material::AIR
        && getBlock(level, 2, -1, -1, box)->getBlock()->mMaterial != Material::AIR)
    {
        placeBlock(level, stairsNorth, 2, 0, -1, box);
        if (getBlock(level, 2, -1, -1, box)->getBlock() == Blocks::grass_path)
            placeBlock(level, Blocks::grass->defaultBlockState(), 2, -1, -1, box);
    }

    for (int z = 0; z < 9; ++z)
    {
        for (int x = 0; x < 5; ++x)
        {
            generateAirColumnUp(level, x, 12, z, box);
            fillColumnDown(level, cobblestone, x, -1, z, box);
        }
    }

    spawnVillagers(level, box, 2, 1, 2, 1);
    return true;
}

int VillagePieces::SmallTemple::getVillagerProfession(int x, int z)
{
    return 2;
}

StructurePiece* VillagePieces::SmallTemple::Create()
{
    return new SmallTemple();
}

EStructurePiece VillagePieces::SmallTemple::GetType()
{
    return eStructurePiece_SmallTemple;
}

VillagePieces::BookHouse::BookHouse() : VillagePiece()
{
}

VillagePieces::BookHouse::BookHouse(StartPiece* startPiece, int depth, Random& random, BoundingBox* box,
                                    const Direction* direction)
    : VillagePiece(startPiece, depth)
{
    setOrientation(direction);
    mBoundingBox = box;
}

StructurePiece* VillagePieces::BookHouse::createPiece(StartPiece* startPiece, std::vector<StructurePiece*>* pieces,
                                                      Random& random, int x, int y, int z, const Direction* direction,
                                                      int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, 0, 0, 0, 9, 9, 6, direction);
    if (VillagePiece::isOkBox(box, startPiece) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new BookHouse(startPiece, depth, random, box, direction);
    delete box;
    return NULL;
}

bool VillagePieces::BookHouse::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (mAverageGroundHeight < 0)
    {
        mAverageGroundHeight = getAverageGroundHeight(level, box);
        if (mAverageGroundHeight < 0)
            return true;
        mBoundingBox->move(0, mAverageGroundHeight - mBoundingBox->y1 + 8, 0);
    }

    const BlockState* cobblestone = biomeBlock(Blocks::cobblestone->defaultBlockState());
    const BlockState* stairsNorth = biomeBlock(
        Blocks::oak_stairs->defaultBlockState()->setValue(StairBlock::FACING, Direction::NORTH));
    const BlockState* stairsSouth = biomeBlock(
        Blocks::oak_stairs->defaultBlockState()->setValue(StairBlock::FACING, Direction::SOUTH));
    const BlockState* stairsEast = biomeBlock(
        Blocks::oak_stairs->defaultBlockState()->setValue(StairBlock::FACING, Direction::EAST));
    const BlockState* planks = biomeBlock(Blocks::planks->defaultBlockState());
    const BlockState* stoneStairsNorth = biomeBlock(
        Blocks::stone_stairs->defaultBlockState()->setValue(StairBlock::FACING, Direction::NORTH));
    const BlockState* fence = biomeBlock(Blocks::fence->defaultBlockState());
    Random selectorRandom = random;

    generateBox(level, box, 1, 1, 1, 7, 5, 4, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 0, 0, 0, 8, 0, 5, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 0, 5, 0, 8, 5, 5, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 0, 6, 1, 8, 6, 4, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 0, 7, 2, 8, 7, 3, false, selectorRandom, mCobblestoneSelector);
    for (int i = -1; i <= 2; ++i)
    {
        for (int x = 0; x <= 8; ++x)
        {
            placeBlock(level, stairsNorth, x, 6 + i, i, box);
            placeBlock(level, stairsSouth, x, 6 + i, 5 - i, box);
        }
    }
    if (mIsZombieVillage)
    {
        placeBlock(level, Blocks::web->defaultBlockState(), 4, 4, -1, box);
        placeBlock(level, Blocks::web->defaultBlockState(), 2, 4, 6, box);
    }

    generateBox(level, box, 0, 1, 0, 0, 1, 5, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 1, 1, 5, 8, 1, 5, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 8, 1, 0, 8, 1, 4, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 2, 1, 0, 7, 1, 0, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 0, 2, 0, 0, 4, 0, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 0, 2, 5, 0, 4, 5, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 8, 2, 5, 8, 4, 5, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 8, 2, 0, 8, 4, 0, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 0, 2, 1, 0, 4, 4, planks, planks, false);
    generateBox(level, box, 1, 2, 5, 7, 4, 5, planks, planks, false);
    generateBox(level, box, 8, 2, 1, 8, 4, 4, planks, planks, false);
    generateBox(level, box, 1, 2, 0, 7, 4, 0, planks, planks, false);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 4, 2, 0, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 5, 2, 0, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 6, 2, 0, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 4, 3, 0, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 5, 3, 0, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 6, 3, 0, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 0, 2, 2, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 0, 2, 3, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 0, 3, 2, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 0, 3, 3, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 8, 2, 2, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 8, 2, 3, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 8, 3, 2, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 8, 3, 3, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 2, 2, 5, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 3, 2, 5, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 5, 2, 5, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 6, 2, 5, box);
    generateBox(level, box, 1, 4, 1, 7, 4, 1, planks, planks, false);
    generateBox(level, box, 1, 4, 4, 7, 4, 4, planks, planks, false);
    generateBox(level, box, 1, 3, 4, 7, 3, 4, Blocks::bookshelf->defaultBlockState(),
                Blocks::bookshelf->defaultBlockState(), false);
    if (mIsZombieVillage)
    {
        placeBlock(level, Blocks::web->defaultBlockState(), 1, 4, 2, box);
        placeBlock(level, Blocks::web->defaultBlockState(), 7, 4, 2, box);
    }

    placeBlock(level, planks, 7, 1, 4, box);
    placeBlock(level, stairsEast, 7, 1, 3, box);
    placeBlock(level, stairsNorth, 6, 1, 4, box);
    placeBlock(level, stairsNorth, 5, 1, 4, box);
    placeBlock(level, stairsNorth, 4, 1, 4, box);
    placeBlock(level, stairsNorth, 3, 1, 4, box);
    placeBlock(level, fence, 6, 1, 3, box);
    placeBlock(level, Blocks::carpet->getBlockState(DyeColor::BROWN->getBlockData()), 6, 2, 3, box);
    placeBlock(level, fence, 4, 1, 3, box);
    placeBlock(level, Blocks::carpet->getBlockState(DyeColor::BROWN->getBlockData()), 4, 2, 3, box);
    placeBlock(level, Blocks::crafting_table->defaultBlockState(), 7, 1, 1, box);
    placeBlock(level, Blocks::air->defaultBlockState(), 1, 1, 0, box);
    placeBlock(level, Blocks::air->defaultBlockState(), 1, 2, 0, box);
    createVillageDoor(level, box, random, 1, 1, 0, Direction::NORTH);
    if (getBlock(level, 1, 0, -1, box)->getBlock()->mMaterial == Material::AIR
        && getBlock(level, 1, -1, -1, box)->getBlock()->mMaterial != Material::AIR)
    {
        placeBlock(level, stoneStairsNorth, 1, 0, -1, box);
        if (getBlock(level, 1, -1, -1, box)->getBlock() == Blocks::grass_path)
            placeBlock(level, Blocks::grass->defaultBlockState(), 1, -1, -1, box);
    }

    for (int z = 0; z < 6; ++z)
    {
        for (int x = 0; x < 9; ++x)
        {
            generateAirColumnUp(level, x, 9, z, box);
            fillColumnDown(level, cobblestone, x, -1, z, box);
        }
    }

    spawnVillagers(level, box, 2, 1, 2, 1);
    return true;
}

int VillagePieces::BookHouse::getVillagerProfession(int x, int z)
{
    return 1;
}

StructurePiece* VillagePieces::BookHouse::Create()
{
    return new BookHouse();
}

EStructurePiece VillagePieces::BookHouse::GetType()
{
    return eStructurePiece_BookHouse;
}

VillagePieces::SmallHut::SmallHut() : VillagePiece()
{
}

VillagePieces::SmallHut::SmallHut(StartPiece* startPiece, int depth, Random& random, BoundingBox* box,
                                  const Direction* direction)
    : VillagePiece(startPiece, depth)
{
    setOrientation(direction);
    mBoundingBox = box;
    mIsTallHouse = random.nextBoolean();
    mTablePosition = random.nextInt(3);
}

StructurePiece* VillagePieces::SmallHut::createPiece(StartPiece* startPiece, std::vector<StructurePiece*>* pieces,
                                                     Random& random, int x, int y, int z, const Direction* direction,
                                                     int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, 0, 0, 0, 4, 6, 5, direction);
    if (isOkBox(box, startPiece) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new SmallHut(startPiece, depth, random, box, direction);
    delete box;
    return NULL;
}

bool VillagePieces::SmallHut::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (mAverageGroundHeight < 0)
    {
        mAverageGroundHeight = getAverageGroundHeight(level, box);
        if (mAverageGroundHeight < 0)
            return true;
        mBoundingBox->move(0, mAverageGroundHeight - mBoundingBox->y1 + 5, 0);
    }

    const BlockState* cobblestone = biomeBlock(Blocks::cobblestone->defaultBlockState());
    const BlockState* planks = biomeBlock(Blocks::planks->defaultBlockState());
    const BlockState* stairsNorth = biomeBlock(
        Blocks::stone_stairs->defaultBlockState()->setValue(StairBlock::FACING, Direction::NORTH));
    const BlockState* log = biomeBlock(Blocks::log->defaultBlockState());
    const BlockState* fence = biomeBlock(Blocks::fence->defaultBlockState());
    Random selectorRandom = random;

    generateBox(level, box, 1, 1, 1, 3, 5, 4, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 0, 0, 0, 3, 0, 4, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 1, 0, 1, 2, 0, 3, Blocks::dirt->defaultBlockState(), Blocks::dirt->defaultBlockState(),
                false);
    if (mIsTallHouse)
    {
        generateBox(level, box, 1, 4, 1, 2, 4, 3, log, log, false);
        if (mIsZombieVillage)
        {
            int webX = random.nextBoolean() ? 1 : 2;
            int webZ = random.nextBoolean() ? 1 : 3;
            placeBlock(level, Blocks::web->defaultBlockState(), webX, 3, webZ, box);
        }
    }
    else
    {
        generateBox(level, box, 1, 5, 1, 2, 5, 3, log, log, false);
        if (mIsZombieVillage)
        {
            int webX = random.nextBoolean() ? 1 : 2;
            int webZ = random.nextBoolean() ? 1 : 3;
            placeBlock(level, Blocks::web->defaultBlockState(), webX, 4, webZ, box);
        }
    }

    placeBlock(level, log, 1, 4, 0, box);
    placeBlock(level, log, 2, 4, 0, box);
    placeBlock(level, log, 1, 4, 4, box);
    placeBlock(level, log, 2, 4, 4, box);
    placeBlock(level, log, 0, 4, 1, box);
    placeBlock(level, log, 0, 4, 2, box);
    placeBlock(level, log, 0, 4, 3, box);
    placeBlock(level, log, 3, 4, 1, box);
    placeBlock(level, log, 3, 4, 2, box);
    placeBlock(level, log, 3, 4, 3, box);
    generateBox(level, box, 0, 1, 0, 0, 3, 0, log, log, false);
    generateBox(level, box, 3, 1, 0, 3, 3, 0, log, log, false);
    generateBox(level, box, 0, 1, 4, 0, 3, 4, log, log, false);
    generateBox(level, box, 3, 1, 4, 3, 3, 4, log, log, false);
    generateBox(level, box, 0, 1, 1, 0, 3, 3, planks, planks, false);
    generateBox(level, box, 3, 1, 1, 3, 3, 3, planks, planks, false);
    generateBox(level, box, 1, 1, 0, 2, 3, 0, planks, planks, false);
    generateBox(level, box, 1, 1, 4, 2, 3, 4, planks, planks, false);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 0, 2, 2, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 3, 2, 2, box);
    if (mTablePosition > 0)
    {
        placeBlock(level, fence, mTablePosition, 1, 3, box);
        placeBlock(level, Blocks::carpet->getBlockState(DyeColor::BROWN->getBlockData()), mTablePosition, 2, 3, box);
    }

    placeBlock(level, Blocks::air->defaultBlockState(), 1, 1, 0, box);
    placeBlock(level, Blocks::air->defaultBlockState(), 1, 2, 0, box);
    createVillageDoor(level, box, random, 1, 1, 0, Direction::NORTH);
    if (getBlock(level, 1, 0, -1, box)->getBlock()->mMaterial == Material::AIR
        && getBlock(level, 1, -1, -1, box)->getBlock()->mMaterial != Material::AIR)
    {
        placeBlock(level, stairsNorth, 1, 0, -1, box);
        if (getBlock(level, 1, -1, -1, box)->getBlock() == Blocks::grass_path)
            placeBlock(level, Blocks::grass->defaultBlockState(), 1, -1, -1, box);
    }

    for (int z = 0; z < 5; ++z)
    {
        for (int x = 0; x < 4; ++x)
        {
            generateAirColumnUp(level, x, 6, z, box);
            fillColumnDown(level, cobblestone, x, -1, z, box);
        }
    }

    spawnVillagers(level, box, 1, 1, 2, 1);
    return true;
}

StructurePiece* VillagePieces::SmallHut::Create()
{
    return new SmallHut();
}

EStructurePiece VillagePieces::SmallHut::GetType()
{
    return eStructurePiece_SmallHut;
}

void VillagePieces::SmallHut::addAdditionalSaveData(DataOutputStream* output)
{
    VillagePiece::addAdditionalSaveData(output);
    output->writeInt(mTablePosition);
    output->writeBoolean(mIsTallHouse);
}

void VillagePieces::SmallHut::readAdditionalSaveData(DataInputStream* input, StructureManager* manager)
{
    VillagePiece::readAdditionalSaveData(input, manager);
    if (StructureFeatureIO::getVersion() == 1)
    {
        std::vector<int>::iterator first = mFirstInts.begin();
        mTablePosition = &*first != NULL && static_cast<unsigned int>(mFirstInts.end() - first) > 0 ? first[0] : 0;
        mIsTallHouse = !mSecondBools.empty() ? mSecondBools[0] : false;
    }
    else
    {
        mTablePosition = input->readInt();
        mIsTallHouse = input->readBoolean();
    }
}

VillagePieces::PigHouse::PigHouse() : VillagePiece()
{
}

VillagePieces::PigHouse::PigHouse(StartPiece* startPiece, int depth, Random& random, BoundingBox* box,
                                  const Direction* direction)
    : VillagePiece(startPiece, depth)
{
    setOrientation(direction);
    mBoundingBox = box;
}

StructurePiece* VillagePieces::PigHouse::createPiece(StartPiece* startPiece, std::vector<StructurePiece*>* pieces,
                                                     Random& random, int x, int y, int z, const Direction* direction,
                                                     int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, 0, 0, 0, 9, 7, 11, direction);
    if (isOkBox(box, startPiece) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new PigHouse(startPiece, depth, random, box, direction);
    delete box;
    return NULL;
}

bool VillagePieces::PigHouse::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (mAverageGroundHeight < 0)
    {
        mAverageGroundHeight = getAverageGroundHeight(level, box);
        if (mAverageGroundHeight < 0)
            return true;
        mBoundingBox->move(0, mAverageGroundHeight - mBoundingBox->y1 + 6, 0);
    }

    const BlockState* cobblestone = biomeBlock(Blocks::cobblestone->defaultBlockState());
    const BlockState* stairsNorth = biomeBlock(
        Blocks::oak_stairs->defaultBlockState()->setValue(StairBlock::FACING, Direction::NORTH));
    const BlockState* stairsSouth = biomeBlock(
        Blocks::oak_stairs->defaultBlockState()->setValue(StairBlock::FACING, Direction::SOUTH));
    const BlockState* stairsWest = biomeBlock(
        Blocks::oak_stairs->defaultBlockState()->setValue(StairBlock::FACING, Direction::WEST));
    const BlockState* planks = biomeBlock(Blocks::planks->defaultBlockState());
    const BlockState* log = biomeBlock(Blocks::log->defaultBlockState());
    const BlockState* fence = biomeBlock(Blocks::fence->defaultBlockState());
    Random selectorRandom = random;

    generateBox(level, box, 1, 1, 1, 7, 4, 4, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 2, 1, 6, 8, 4, 10, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 2, 0, 6, 8, 0, 10, Blocks::dirt->defaultBlockState(), Blocks::dirt->defaultBlockState(),
                false);
    placeBlock(level, selectorRandom, cobblestone, mCobblestoneSelector, 6, 0, 6, box);
    generateBox(level, box, 2, 1, 6, 2, 1, 10, fence, fence, false);
    generateBox(level, box, 8, 1, 6, 8, 1, 10, fence, fence, false);
    generateBox(level, box, 3, 1, 10, 7, 1, 10, fence, fence, false);
    generateBox(level, box, 1, 0, 1, 7, 0, 4, planks, planks, false);
    generateBox(level, box, 0, 0, 0, 0, 3, 5, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 8, 0, 0, 8, 3, 5, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 1, 0, 0, 7, 1, 0, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 1, 0, 5, 7, 1, 5, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 1, 2, 0, 7, 3, 0, planks, planks, false);
    generateBox(level, box, 1, 2, 5, 7, 3, 5, planks, planks, false);
    generateBox(level, box, 0, 4, 1, 8, 4, 1, planks, planks, false);
    generateBox(level, box, 0, 4, 4, 8, 4, 4, planks, planks, false);
    generateBox(level, box, 0, 5, 2, 8, 5, 3, planks, planks, false);
    placeBlock(level, planks, 0, 4, 2, box);
    placeBlock(level, planks, 0, 4, 3, box);
    placeBlock(level, planks, 8, 4, 2, box);
    placeBlock(level, planks, 8, 4, 3, box);
    for (int i = -1; i <= 2; ++i)
    {
        for (int x = 0; x <= 8; ++x)
        {
            placeBlock(level, stairsNorth, x, 4 + i, i, box);
            placeBlock(level, stairsSouth, x, 4 + i, 5 - i, box);
        }
    }

    placeBlock(level, log, 0, 2, 1, box);
    placeBlock(level, log, 0, 2, 4, box);
    placeBlock(level, log, 8, 2, 1, box);
    placeBlock(level, log, 8, 2, 4, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 0, 2, 2, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 0, 2, 3, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 8, 2, 2, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 8, 2, 3, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 2, 2, 5, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 3, 2, 5, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 5, 2, 0, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 6, 2, 5, box);
    placeBlock(level, fence, 2, 1, 3, box);
    placeBlock(level, Blocks::carpet->getBlockState(DyeColor::BROWN->getBlockData()), 2, 2, 3, box);
    placeBlock(level, planks, 1, 1, 4, box);
    placeBlock(level, stairsNorth, 2, 1, 4, box);
    placeBlock(level, stairsWest, 1, 1, 3, box);
    generateBox(level, box, 5, 0, 1, 7, 0, 3, Blocks::double_stone_slab->defaultBlockState(),
                Blocks::double_stone_slab->defaultBlockState(), false);
    placeBlock(level, Blocks::double_stone_slab->defaultBlockState(), 6, 1, 1, box);
    placeBlock(level, Blocks::double_stone_slab->defaultBlockState(), 6, 1, 2, box);
    placeBlock(level, Blocks::air->defaultBlockState(), 2, 1, 0, box);
    placeBlock(level, Blocks::air->defaultBlockState(), 2, 2, 0, box);
    placeTorch(level, Direction::NORTH, 2, 3, 1, box);
    createVillageDoor(level, box, random, 2, 1, 0, Direction::NORTH);
    if (getBlock(level, 2, 0, -1, box)->getBlock()->mMaterial == Material::AIR
        && getBlock(level, 2, -1, -1, box)->getBlock()->mMaterial != Material::AIR)
    {
        placeBlock(level, stairsNorth, 2, 0, -1, box);
        if (getBlock(level, 2, -1, -1, box)->getBlock() == Blocks::grass_path)
            placeBlock(level, Blocks::grass->defaultBlockState(), 2, -1, -1, box);
    }

    placeBlock(level, Blocks::air->defaultBlockState(), 6, 1, 5, box);
    placeBlock(level, Blocks::air->defaultBlockState(), 6, 2, 5, box);
    placeTorch(level, Direction::SOUTH, 6, 3, 4, box);
    createVillageDoor(level, box, random, 6, 1, 5, Direction::SOUTH);

    for (int z = 0; z < 5; ++z)
    {
        for (int x = 0; x < 9; ++x)
        {
            generateAirColumnUp(level, x, 7, z, box);
            fillColumnDown(level, cobblestone, x, -1, z, box);
        }
    }

    spawnVillagers(level, box, 4, 1, 2, 2);
    return true;
}

int VillagePieces::PigHouse::getVillagerProfession(int index, int profession)
{
    if (index == 0)
        return 4;
    return VillagePiece::getVillagerProfession(index, profession);
}

StructurePiece* VillagePieces::PigHouse::Create()
{
    return new PigHouse();
}

EStructurePiece VillagePieces::PigHouse::GetType()
{
    return eStructurePiece_PigHouse;
}

VillagePieces::TwoRoomHouse::TwoRoomHouse() : VillagePiece()
{
}

VillagePieces::TwoRoomHouse::TwoRoomHouse(StartPiece* startPiece, int depth, Random& random, BoundingBox* box,
                                          const Direction* direction)
    : VillagePiece(startPiece, depth),
      mHasPlacedChest(false)
{
    setOrientation(direction);
    mBoundingBox = box;
}

StructurePiece* VillagePieces::TwoRoomHouse::createPiece(StartPiece* startPiece, std::vector<StructurePiece*>* pieces,
                                                         Random& random, int x, int y, int z,
                                                         const Direction* direction, int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, 0, 0, 0, 9, 7, 12, direction);
    if (isOkBox(box, startPiece) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new TwoRoomHouse(startPiece, depth, random, box, direction);
    delete box;
    return NULL;
}

bool VillagePieces::TwoRoomHouse::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (mAverageGroundHeight < 0)
    {
        mAverageGroundHeight = getAverageGroundHeight(level, box);
        if (mAverageGroundHeight < 0)
            return true;
        mBoundingBox->move(0, mAverageGroundHeight - mBoundingBox->y1 + 6, 0);
    }

    const BlockState* cobblestone = biomeBlock(Blocks::cobblestone->defaultBlockState());
    const BlockState* stairsNorth = biomeBlock(
        Blocks::oak_stairs->defaultBlockState()->setValue(StairBlock::FACING, Direction::NORTH));
    const BlockState* stairsSouth = biomeBlock(
        Blocks::oak_stairs->defaultBlockState()->setValue(StairBlock::FACING, Direction::SOUTH));
    const BlockState* stairsEast = biomeBlock(
        Blocks::oak_stairs->defaultBlockState()->setValue(StairBlock::FACING, Direction::EAST));
    const BlockState* stairsWest = biomeBlock(
        Blocks::oak_stairs->defaultBlockState()->setValue(StairBlock::FACING, Direction::WEST));
    const BlockState* planks = biomeBlock(Blocks::planks->defaultBlockState());
    const BlockState* log = biomeBlock(Blocks::log->defaultBlockState());
    Random selectorRandom = random;

    generateBox(level, box, 1, 1, 1, 7, 4, 4, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 2, 1, 6, 8, 4, 10, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 2, 0, 5, 8, 0, 10, planks, planks, false);
    generateBox(level, box, 1, 0, 1, 7, 0, 4, planks, planks, false);
    generateBox(level, box, 0, 0, 0, 0, 3, 5, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 8, 0, 0, 8, 3, 10, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 1, 0, 0, 7, 2, 0, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 1, 0, 5, 2, 1, 5, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 2, 0, 6, 2, 3, 10, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 3, 0, 10, 7, 3, 10, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 1, 2, 0, 7, 3, 0, planks, planks, false);
    generateBox(level, box, 1, 2, 5, 2, 3, 5, planks, planks, false);
    generateBox(level, box, 0, 4, 1, 8, 4, 1, planks, planks, false);
    generateBox(level, box, 0, 4, 4, 3, 4, 4, planks, planks, false);
    generateBox(level, box, 0, 5, 2, 8, 5, 3, planks, planks, false);
    placeBlock(level, planks, 0, 4, 2, box);
    placeBlock(level, planks, 0, 4, 3, box);
    placeBlock(level, planks, 8, 4, 2, box);
    placeBlock(level, planks, 8, 4, 3, box);
    placeBlock(level, planks, 8, 4, 4, box);
    for (int i = -1; i <= 2; ++i)
    {
        for (int x = 0; x <= 8; ++x)
        {
            placeBlock(level, stairsNorth, x, 4 + i, i, box);
            if ((i > -1 || x <= 1) && (i > 0 || x <= 3) && (i > 1 || x <= 4 || x >= 6))
                placeBlock(level, stairsSouth, x, 4 + i, 5 - i, box);
        }
    }

    generateBox(level, box, 3, 4, 5, 3, 4, 10, planks, planks, false);
    generateBox(level, box, 7, 4, 2, 7, 4, 10, planks, planks, false);
    generateBox(level, box, 4, 5, 4, 4, 5, 10, planks, planks, false);
    generateBox(level, box, 6, 5, 4, 6, 5, 10, planks, planks, false);
    generateBox(level, box, 5, 6, 3, 5, 6, 10, planks, planks, false);
    if (mIsZombieVillage)
    {
        placeBlock(level, Blocks::web->defaultBlockState(), 5, 5, 3, box);
        placeBlock(level, Blocks::web->defaultBlockState(), 5, 5, 4, box);
        placeBlock(level, Blocks::web->defaultBlockState(), 5, 5, 10, box);
    }

    for (int x = 4; x >= 1; --x)
    {
        placeBlock(level, planks, x, 2 + x, 7 - x, box);
        for (int z = 8 - x; z <= 10; ++z)
            placeBlock(level, stairsEast, x, 2 + x, z, box);
    }

    placeBlock(level, planks, 6, 6, 3, box);
    placeBlock(level, planks, 7, 5, 4, box);
    placeBlock(level, stairsWest, 6, 6, 4, box);
    for (int x = 6; x <= 8; ++x)
    {
        for (int z = 5; z <= 10; ++z)
            placeBlock(level, stairsWest, x, 12 - x, z, box);
    }

    placeBlock(level, log, 0, 2, 1, box);
    placeBlock(level, log, 0, 2, 4, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 0, 2, 2, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 0, 2, 3, box);
    placeBlock(level, log, 4, 2, 0, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 5, 2, 0, box);
    placeBlock(level, log, 6, 2, 0, box);
    placeBlock(level, log, 8, 2, 1, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 8, 2, 2, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 8, 2, 3, box);
    placeBlock(level, log, 8, 2, 4, box);
    placeBlock(level, planks, 8, 2, 5, box);
    placeBlock(level, log, 8, 2, 6, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 8, 2, 7, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 8, 2, 8, box);
    placeBlock(level, log, 8, 2, 9, box);
    placeBlock(level, log, 2, 2, 6, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 2, 2, 7, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 2, 2, 8, box);
    placeBlock(level, log, 2, 2, 9, box);
    placeBlock(level, log, 4, 4, 10, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 5, 4, 10, box);
    placeBlock(level, log, 6, 4, 10, box);
    placeBlock(level, planks, 5, 5, 10, box);
    placeBlock(level, Blocks::air->defaultBlockState(), 2, 1, 0, box);
    placeBlock(level, Blocks::air->defaultBlockState(), 2, 2, 0, box);
    placeTorch(level, Direction::NORTH, 2, 3, 1, box);
    createVillageDoor(level, box, random, 2, 1, 0, Direction::NORTH);
    generateBox(level, box, 1, 0, -1, 3, 2, -1, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    if (getBlock(level, 2, 0, -1, box)->getBlock()->mMaterial == Material::AIR
        && getBlock(level, 2, -1, -1, box)->getBlock()->mMaterial != Material::AIR)
    {
        placeBlock(level, stairsNorth, 2, 0, -1, box);
        if (getBlock(level, 2, -1, -1, box)->getBlock() == Blocks::grass_path)
            placeBlock(level, Blocks::grass->defaultBlockState(), 2, -1, -1, box);
    }

    for (int z = 0; z < 5; ++z)
    {
        for (int x = 0; x < 9; ++x)
        {
            generateAirColumnUp(level, x, 7, z, box);
            fillColumnDown(level, cobblestone, x, -1, z, box);
        }
    }
    for (int z = 5; z < 11; ++z)
    {
        for (int x = 2; x < 9; ++x)
        {
            generateAirColumnUp(level, x, 7, z, box);
            fillColumnDown(level, cobblestone, x, -1, z, box);
        }
    }

    Biome* biome = level->getBiome(BlockPos(box->getCenter().x, box->getCenter().y, box->getCenter().z));
    if (biome->isSnowCovered() && !mHasPlacedChest)
    {
        BlockPos chestPos(getWorldX(5, 9), getWorldY(1), getWorldZ(5, 9));
        if (box->isInside(chestPos))
        {
            setDirty();
            mHasPlacedChest = true;
            createChest(level, box, random, 5, 1, 9, &BuiltInLootTables::VILLAGE_TWO_ROOM_HOUSE);
        }
    }

    spawnVillagers(level, box, 4, 1, 2, 2);
    return true;
}

StructurePiece* VillagePieces::TwoRoomHouse::Create()
{
    return new TwoRoomHouse();
}

EStructurePiece VillagePieces::TwoRoomHouse::GetType()
{
    return eStructurePiece_TwoRoomHouse;
}

VillagePieces::Smithy::Smithy() : VillagePiece()
{
}

VillagePieces::Smithy::Smithy(StartPiece* startPiece, int depth, Random& random, BoundingBox* box,
                              const Direction* direction)
    : VillagePiece(startPiece, depth),
      mHasPlacedChest(false)
{
    setOrientation(direction);
    mBoundingBox = box;
}

StructurePiece* VillagePieces::Smithy::createPiece(StartPiece* startPiece, std::vector<StructurePiece*>* pieces,
                                                   Random& random, int x, int y, int z, const Direction* direction,
                                                   int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, 0, 0, 0, 10, 6, 7, direction);
    if (isOkBox(box, startPiece) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new Smithy(startPiece, depth, random, box, direction);
    delete box;
    return NULL;
}

bool VillagePieces::Smithy::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (mAverageGroundHeight < 0)
    {
        mAverageGroundHeight = getAverageGroundHeight(level, box);
        if (mAverageGroundHeight < 0)
            return true;
        mBoundingBox->move(0, mAverageGroundHeight - mBoundingBox->y1 + 5, 0);
    }

    const BlockState* cobblestone = biomeBlock(Blocks::cobblestone->defaultBlockState());
    const BlockState* stairsNorth = biomeBlock(
        Blocks::oak_stairs->defaultBlockState()->setValue(StairBlock::FACING, Direction::NORTH));
    const BlockState* stairsWest = biomeBlock(
        Blocks::oak_stairs->defaultBlockState()->setValue(StairBlock::FACING, Direction::WEST));
    const BlockState* planks = biomeBlock(Blocks::planks->defaultBlockState());
    const BlockState* stoneStairsNorth = biomeBlock(
        Blocks::stone_stairs->defaultBlockState()->setValue(StairBlock::FACING, Direction::NORTH));
    const BlockState* log = biomeBlock(Blocks::log->defaultBlockState());
    const BlockState* fence = biomeBlock(Blocks::fence->defaultBlockState());
    Random selectorRandom = random;

    generateBox(level, box, 0, 1, 0, 9, 4, 6, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 0, 0, 0, 9, 0, 6, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 0, 4, 0, 9, 4, 6, false, selectorRandom, mCobblestoneSelector);
    generateBox(level, box, 0, 5, 0, 9, 5, 6, Blocks::stone_slab->defaultBlockState(),
                Blocks::stone_slab->defaultBlockState(), false);
    generateBox(level, box, 1, 5, 1, 8, 5, 5, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 1, 1, 0, 2, 3, 0, planks, planks, false);
    generateBox(level, box, 0, 1, 0, 0, 4, 0, log, log, false);
    generateBox(level, box, 3, 1, 0, 3, 4, 0, log, log, false);
    generateBox(level, box, 0, 1, 6, 0, 4, 6, log, log, false);
    placeBlock(level, planks, 3, 3, 1, box);
    generateBox(level, box, 3, 1, 2, 3, 3, 2, planks, planks, false);
    generateBox(level, box, 4, 1, 3, 5, 3, 3, planks, planks, false);
    generateBox(level, box, 0, 1, 1, 0, 3, 5, planks, planks, false);
    generateBox(level, box, 1, 1, 6, 5, 3, 6, planks, planks, false);
    generateBox(level, box, 5, 1, 0, 5, 3, 0, fence, fence, false);
    generateBox(level, box, 9, 1, 0, 9, 3, 0, fence, fence, false);
    generateBox(level, box, 6, 1, 4, 9, 4, 6, false, selectorRandom, mCobblestoneSelector);
    placeBlock(level, Blocks::flowing_lava->defaultBlockState(), 7, 1, 5, box);
    placeBlock(level, Blocks::flowing_lava->defaultBlockState(), 8, 1, 5, box);
    placeBlock(level, Blocks::iron_bars->defaultBlockState(), 9, 2, 5, box);
    placeBlock(level, Blocks::iron_bars->defaultBlockState(), 9, 2, 4, box);
    generateBox(level, box, 7, 2, 4, 8, 2, 5, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    placeBlock(level, selectorRandom, cobblestone, mCobblestoneSelector, 6, 1, 3, box);
    placeBlock(level, Blocks::furnace->defaultBlockState(), 6, 2, 3, box);
    placeBlock(level, Blocks::furnace->defaultBlockState(), 6, 3, 3, box);
    placeBlock(level, Blocks::double_stone_slab->defaultBlockState(), 8, 1, 1, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 0, 2, 2, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 0, 2, 4, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 2, 2, 6, box);
    placeBlock(level, Blocks::glass_pane->defaultBlockState(), 4, 2, 6, box);
    placeBlock(level, fence, 2, 1, 4, box);
    placeBlock(level, Blocks::carpet->getBlockState(DyeColor::BROWN->getBlockData()), 2, 2, 4, box);
    placeBlock(level, planks, 1, 1, 5, box);
    placeBlock(level, stairsNorth, 2, 1, 5, box);
    placeBlock(level, stairsWest, 1, 1, 4, box);
    if (!mHasPlacedChest)
    {
        BlockPos chestPos(getWorldX(5, 5), getWorldY(1), getWorldZ(5, 5));
        if (box->isInside(chestPos))
        {
            setDirty();
            mHasPlacedChest = true;
            createChest(level, box, random, 5, 1, 5, &BuiltInLootTables::VILLAGE_BLACKSMITH);
        }
    }

    for (int x = 6; x <= 8; ++x)
    {
        if (getBlock(level, x, 0, -1, box)->getBlock()->mMaterial == Material::AIR
            && getBlock(level, x, -1, -1, box)->getBlock()->mMaterial != Material::AIR)
        {
            placeBlock(level, stoneStairsNorth, x, 0, -1, box);
            if (getBlock(level, x, -1, -1, box)->getBlock() == Blocks::grass_path)
                placeBlock(level, Blocks::grass->defaultBlockState(), x, -1, -1, box);
        }
    }

    for (int z = 0; z < 7; ++z)
    {
        for (int x = 0; x < 10; ++x)
        {
            generateAirColumnUp(level, x, 6, z, box);
            fillColumnDown(level, cobblestone, x, -1, z, box);
        }
    }

    spawnVillagers(level, box, 7, 1, 1, 1);
    return true;
}

int VillagePieces::Smithy::getVillagerProfession(int x, int z)
{
    return 3;
}

StructurePiece* VillagePieces::Smithy::Create()
{
    return new Smithy();
}

EStructurePiece VillagePieces::Smithy::GetType()
{
    return eStructurePiece_Smithy;
}

void VillagePieces::Smithy::addAdditionalSaveData(DataOutputStream* output)
{
    VillagePiece::addAdditionalSaveData(output);
    output->writeBoolean(mHasPlacedChest);
}

void VillagePieces::Smithy::readAdditionalSaveData(DataInputStream* input, StructureManager* manager)
{
    VillagePiece::readAdditionalSaveData(input, manager);
    mHasPlacedChest = input->readBoolean();
}

void VillagePieces::Smithy::staticCtor()
{
}

VillagePieces::Farmland::Farmland() : VillagePiece()
{
    mCropA = NULL;
    mCropB = NULL;
}

VillagePieces::Farmland::Farmland(StartPiece* startPiece, int depth, Random& random, BoundingBox* box,
                                  const Direction* direction)
    : VillagePiece(startPiece, depth)
{
    setOrientation(direction);
    mBoundingBox = box;
    mCropA = selectCrops(random);
    mCropB = selectCrops(random);
}

Block* VillagePieces::Farmland::selectCrops(Random& random)
{
    switch (random.nextInt(10))
    {
    case 0:
    case 1:
        return Blocks::carrots;
    case 2:
    case 3:
        return Blocks::potatoes;
    case 4:
        return Blocks::beetroots;
    }
    return Blocks::wheat;
}

StructurePiece* VillagePieces::Farmland::createPiece(StartPiece* startPiece, std::vector<StructurePiece*>* pieces,
                                                     Random& random, int x, int y, int z, const Direction* direction,
                                                     int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, 0, 0, 0, 7, 4, 9, direction);
    if (VillagePiece::isOkBox(box, startPiece) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new Farmland(startPiece, depth, random, box, direction);
    delete box;
    return NULL;
}

Block* VillagePieces::Farmland::selectOldCrops(int crop)
{
    switch (crop)
    {
    case 0:
        return Blocks::carrots;
    case 1:
        return Blocks::potatoes;
    default:
        return Blocks::wheat;
    }
}

void VillagePieces::Farmland::addAdditionalSaveData(DataOutputStream* output)
{
    VillagePiece::addAdditionalSaveData(output);
    output->writeInt(Block::blockRegistry->getId(mCropA));
    output->writeInt(Block::blockRegistry->getId(mCropB));
}

void VillagePieces::Farmland::readAdditionalSaveData(DataInputStream* input, StructureManager* manager)
{
    VillagePiece::readAdditionalSaveData(input, manager);
    if (StructureFeatureIO::getVersion() == 1)
    {
        std::vector<int>::iterator first = mFirstInts.begin();
        mCropA = &*first != NULL && static_cast<unsigned int>(mFirstInts.end() - first) > 1 ? selectOldCrops(first[0])
                                                                                            : Blocks::wheat;
        mCropB = &*first != NULL && static_cast<unsigned int>(mFirstInts.end() - first) > 1 ? selectOldCrops(first[1])
                                                                                            : Blocks::wheat;
    }
    else
    {
        mCropA = Block::byId(input->readInt());
        mCropB = Block::byId(input->readInt());
    }
}

bool VillagePieces::Farmland::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (mAverageGroundHeight < 0)
    {
        mAverageGroundHeight = getAverageGroundHeight(level, box);
        if (mAverageGroundHeight < 0)
            return true;
        mBoundingBox->move(0, mAverageGroundHeight - mBoundingBox->y1 + 3, 0);
    }

    const BlockState* log = biomeBlock(Blocks::log->defaultBlockState());
    generateBox(level, box, 0, 1, 0, 6, 4, 8, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    const BlockState* farmland = Blocks::farmland->defaultBlockState();
    Biome* biome = level->getBiome(BlockPos(box->getCenter().x, box->getCenter().y, box->getCenter().z));
    if (biome->isSnowCovered())
        farmland = Blocks::dirt->defaultBlockState();
    generateBox(level, box, 1, 0, 1, 2, 0, 7, farmland, farmland, false);
    generateBox(level, box, 4, 0, 1, 5, 0, 7, farmland, farmland, false);
    generateBox(level, box, 0, 0, 0, 0, 0, 8, log, log, false);
    generateBox(level, box, 6, 0, 0, 6, 0, 8, log, log, false);
    generateBox(level, box, 1, 0, 0, 5, 0, 0, log, log, false);
    generateBox(level, box, 1, 0, 8, 5, 0, 8, log, log, false);
    generateBox(level, box, 3, 0, 1, 3, 0, 7, Blocks::flowing_water->defaultBlockState(),
                Blocks::flowing_water->defaultBlockState(), false);

    if (!biome->isSnowCovered())
    {
        for (int z = 1; z <= 7; ++z)
        {
            int maxAge = dynamic_cast<CropBlock*>(mCropA)->getMaxAge();
            placeBlock(level, mCropA->getBlockState(Mth::nextInt(&random, maxAge / 3, maxAge)), 1, 1, z, box);
            placeBlock(level, mCropA->getBlockState(Mth::nextInt(&random, maxAge / 3, maxAge)), 2, 1, z, box);
            maxAge = dynamic_cast<CropBlock*>(mCropB)->getMaxAge();
            placeBlock(level, mCropB->getBlockState(Mth::nextInt(&random, maxAge / 3, maxAge)), 4, 1, z, box);
            placeBlock(level, mCropB->getBlockState(Mth::nextInt(&random, maxAge / 3, maxAge)), 5, 1, z, box);
        }
    }

    for (int z = 0; z < 9; ++z)
    {
        for (int x = 0; x < 7; ++x)
        {
            generateAirColumnUp(level, x, 4, z, box);
            fillColumnDown(level, Blocks::dirt->defaultBlockState(), x, -1, z, box);
        }
    }
    return true;
}

StructurePiece* VillagePieces::Farmland::Create()
{
    return new Farmland();
}

EStructurePiece VillagePieces::Farmland::GetType()
{
    return eStructurePiece_Farmland;
}

VillagePieces::DoubleFarmland::DoubleFarmland() : VillagePiece()
{
    mCropA = NULL;
    mCropB = NULL;
    mCropC = NULL;
    mCropD = NULL;
}

VillagePieces::DoubleFarmland::DoubleFarmland(StartPiece* startPiece, int depth, Random& random, BoundingBox* box,
                                              const Direction* direction)
    : VillagePiece(startPiece, depth)
{
    setOrientation(direction);
    mBoundingBox = box;
    mCropA = selectCrops(random);
    mCropB = selectCrops(random);
    mCropC = selectCrops(random);
    mCropD = selectCrops(random);
}

Block* VillagePieces::DoubleFarmland::selectCrops(Random& random)
{
    switch (random.nextInt(10))
    {
    case 0:
    case 1:
        return Blocks::carrots;
    case 2:
    case 3:
        return Blocks::potatoes;
    case 4:
        return Blocks::beetroots;
    }
    return Blocks::wheat;
}

StructurePiece* VillagePieces::DoubleFarmland::createPiece(StartPiece* startPiece, std::vector<StructurePiece*>* pieces,
                                                           Random& random, int x, int y, int z,
                                                           const Direction* direction, int depth)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, 0, 0, 0, 13, 4, 9, direction);
    if (VillagePiece::isOkBox(box, startPiece) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return new DoubleFarmland(startPiece, depth, random, box, direction);
    delete box;
    return NULL;
}

Block* VillagePieces::DoubleFarmland::selectOldCrops(int crop)
{
    switch (crop)
    {
    case 0:
        return Blocks::carrots;
    case 1:
        return Blocks::potatoes;
    default:
        return Blocks::wheat;
    }
}

void VillagePieces::DoubleFarmland::addAdditionalSaveData(DataOutputStream* output)
{
    VillagePiece::addAdditionalSaveData(output);
    output->writeInt(Block::blockRegistry->getId(mCropA));
    output->writeInt(Block::blockRegistry->getId(mCropB));
    output->writeInt(Block::blockRegistry->getId(mCropC));
    output->writeInt(Block::blockRegistry->getId(mCropD));
}

void VillagePieces::DoubleFarmland::readAdditionalSaveData(DataInputStream* input, StructureManager* manager)
{
    VillagePiece::readAdditionalSaveData(input, manager);
    if (StructureFeatureIO::getVersion() == 1)
    {
        std::vector<int>::iterator first = mFirstInts.begin();
        mCropA = &*first != NULL && static_cast<unsigned int>(mFirstInts.end() - first) > 3 ? selectOldCrops(first[0])
                                                                                            : Blocks::wheat;
        mCropB = &*first != NULL && static_cast<unsigned int>(mFirstInts.end() - first) > 3 ? selectOldCrops(first[1])
                                                                                            : Blocks::wheat;
        mCropC = &*first != NULL && static_cast<unsigned int>(mFirstInts.end() - first) > 3 ? selectOldCrops(first[2])
                                                                                            : Blocks::wheat;
        mCropD = &*first != NULL && static_cast<unsigned int>(mFirstInts.end() - first) > 3 ? selectOldCrops(first[3])
                                                                                            : Blocks::wheat;
        return;
    }

    mCropA = Block::byId(input->readInt());
    mCropB = Block::byId(input->readInt());
    mCropC = Block::byId(input->readInt());
    mCropD = Block::byId(input->readInt());
    if (mCropA == NULL || dynamic_cast<CropBlock*>(mCropA) == NULL)
        mCropA = Blocks::wheat;
    if (mCropB == NULL || dynamic_cast<CropBlock*>(mCropB) == NULL)
        mCropB = Blocks::carrots;
    if (mCropC == NULL || dynamic_cast<CropBlock*>(mCropC) == NULL)
        mCropC = Blocks::potatoes;
    if (mCropD == NULL || dynamic_cast<CropBlock*>(mCropD) == NULL)
        mCropD = Blocks::beetroots;
}

bool VillagePieces::DoubleFarmland::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (mAverageGroundHeight < 0)
    {
        mAverageGroundHeight = getAverageGroundHeight(level, box);
        if (mAverageGroundHeight < 0)
            return true;
        mBoundingBox->move(0, mAverageGroundHeight - mBoundingBox->y1 + 3, 0);
    }

    const BlockState* log = biomeBlock(Blocks::log->defaultBlockState());
    generateBox(level, box, 0, 1, 0, 12, 4, 8, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    const BlockState* farmland = Blocks::farmland->defaultBlockState();
    Biome* biome = level->getBiome(BlockPos(box->getCenter().x, box->getCenter().y, box->getCenter().z));
    if (biome->isSnowCovered())
        farmland = Blocks::dirt->defaultBlockState();
    generateBox(level, box, 1, 0, 1, 2, 0, 7, farmland, farmland, false);
    generateBox(level, box, 4, 0, 1, 5, 0, 7, farmland, farmland, false);
    generateBox(level, box, 7, 0, 1, 8, 0, 7, farmland, farmland, false);
    generateBox(level, box, 10, 0, 1, 11, 0, 7, farmland, farmland, false);
    generateBox(level, box, 0, 0, 0, 0, 0, 8, log, log, false);
    generateBox(level, box, 6, 0, 0, 6, 0, 8, log, log, false);
    generateBox(level, box, 12, 0, 0, 12, 0, 8, log, log, false);
    generateBox(level, box, 1, 0, 0, 11, 0, 0, log, log, false);
    generateBox(level, box, 1, 0, 8, 11, 0, 8, log, log, false);
    generateBox(level, box, 3, 0, 1, 3, 0, 7, Blocks::flowing_water->defaultBlockState(),
                Blocks::flowing_water->defaultBlockState(), false);
    generateBox(level, box, 9, 0, 1, 9, 0, 7, Blocks::flowing_water->defaultBlockState(),
                Blocks::flowing_water->defaultBlockState(), false);

    if (!biome->isSnowCovered())
    {
        for (int z = 1; z <= 7; ++z)
        {
            int maxAge = dynamic_cast<CropBlock*>(mCropA)->getMaxAge();
            placeBlock(level, mCropA->getBlockState(Mth::nextInt(&random, maxAge / 3, maxAge)), 1, 1, z, box);
            placeBlock(level, mCropA->getBlockState(Mth::nextInt(&random, maxAge / 3, maxAge)), 2, 1, z, box);
            maxAge = dynamic_cast<CropBlock*>(mCropB)->getMaxAge();
            placeBlock(level, mCropB->getBlockState(Mth::nextInt(&random, maxAge / 3, maxAge)), 4, 1, z, box);
            placeBlock(level, mCropB->getBlockState(Mth::nextInt(&random, maxAge / 3, maxAge)), 5, 1, z, box);
            maxAge = dynamic_cast<CropBlock*>(mCropC)->getMaxAge();
            placeBlock(level, mCropC->getBlockState(Mth::nextInt(&random, maxAge / 3, maxAge)), 7, 1, z, box);
            placeBlock(level, mCropC->getBlockState(Mth::nextInt(&random, maxAge / 3, maxAge)), 8, 1, z, box);
            maxAge = dynamic_cast<CropBlock*>(mCropD)->getMaxAge();
            placeBlock(level, mCropD->getBlockState(Mth::nextInt(&random, maxAge / 3, maxAge)), 10, 1, z, box);
            placeBlock(level, mCropD->getBlockState(Mth::nextInt(&random, maxAge / 3, maxAge)), 11, 1, z, box);
        }
    }

    for (int z = 0; z < 9; ++z)
    {
        for (int x = 0; x < 13; ++x)
        {
            generateAirColumnUp(level, x, 4, z, box);
            fillColumnDown(level, Blocks::dirt->defaultBlockState(), x, -1, z, box);
        }
    }
    return true;
}

StructurePiece* VillagePieces::DoubleFarmland::Create()
{
    return new DoubleFarmland();
}

EStructurePiece VillagePieces::DoubleFarmland::GetType()
{
    return eStructurePiece_DoubleFarmland;
}

VillagePieces::LightPost::LightPost() : VillagePiece()
{
}

VillagePieces::LightPost::LightPost(StartPiece* startPiece, int depth, Random& random, BoundingBox* box,
                                    const Direction* direction)
    : VillagePiece(startPiece, depth)
{
    setOrientation(direction);
    mBoundingBox = box;
}

BoundingBox* VillagePieces::LightPost::findPieceBox(StartPiece* startPiece, std::vector<StructurePiece*>* pieces,
                                                    Random& random, int x, int y, int z, const Direction* direction)
{
    BoundingBox* box = BoundingBox::orientBox(x, y, z, 0, 0, 0, 3, 4, 2, direction);
    if (isOkBox(box, startPiece) && StructurePiece::findCollisionPiece(pieces, box) == NULL)
        return box;
    delete box;
    return NULL;
}

bool VillagePieces::LightPost::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (mAverageGroundHeight < 0)
    {
        mAverageGroundHeight = getAverageGroundHeight(level, box);
        if (mAverageGroundHeight < 0)
            return true;
        mBoundingBox->move(0, mAverageGroundHeight - mBoundingBox->y1 + 3, 0);
    }

    const BlockState* fence = biomeBlock(Blocks::fence->defaultBlockState());
    generateBox(level, box, 0, 0, 0, 2, 3, 1, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    placeBlock(level, fence, 1, 0, 0, box);
    placeBlock(level, fence, 1, 1, 0, box);
    placeBlock(level, fence, 1, 2, 0, box);
    placeBlock(level, Blocks::wool->getBlockState(DyeColor::BLACK->getBlockData()), 1, 3, 0, box);
    placeTorch(level, Direction::EAST, 2, 3, 0, box);
    placeTorch(level, Direction::NORTH, 1, 3, 1, box);
    placeTorch(level, Direction::WEST, 0, 3, 0, box);
    placeTorch(level, Direction::SOUTH, 1, 3, -1, box);
    return true;
}

StructurePiece* VillagePieces::LightPost::Create()
{
    return new LightPost();
}

EStructurePiece VillagePieces::LightPost::GetType()
{
    return eStructurePiece_LightPost;
}

VillagePieces::CobbleStoneSelector::CobbleStoneSelector(bool mossy, int type) : mMossy(mossy), mType(type)
{
    mNext = Blocks::cobblestone->defaultBlockState();
}

void VillagePieces::CobbleStoneSelector::next(Random& random, int x, int y, int z, bool edge)
{
    if (mMossy && random.nextFloat() >= 0.4f)
    {
        if (mType == 1)
        {
            mNext = Blocks::sandstone->getBlockState(SandStoneBlock::SANDSTONE->getData());
        }
        else
        {
            mNext = Blocks::mossy_cobblestone->defaultBlockState();
        }
    }
    else if (mType == 1)
    {
        mNext = Blocks::sandstone->getBlockState(SandStoneBlock::SANDSTONE->getData());
    }
    else
    {
        mNext = Blocks::cobblestone->defaultBlockState();
    }
}

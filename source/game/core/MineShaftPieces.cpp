#include "game/core/MineShaftPieces.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/BuiltInLootTables.hpp"
#include "game/core/Direction.hpp"
#include "game/core/MinecartChest.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/StructureFeatureIO.hpp"
#include "game/world/block/BaseRailBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/RailBlock.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MobSpawnerBlockEntity.hpp"
#include "game/world/block/TorchBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

MineShaftPieces::MineShaftPiece::MineShaftPiece(int genDepth, MineShaftFeature::MineShaftType type)
    : StructurePiece(genDepth),
      mType(type)
{
}

MineShaftPieces::MineShaftPiece::MineShaftPiece() : StructurePiece()
{
}

const BlockState* MineShaftPieces::MineShaftPiece::getPlanksBlock()
{
    MineShaftFeature::MineShaftType type = mType;
    const BlockState* planks = Blocks::planks->defaultBlockState();
    if (type == MineShaftFeature::eMineShaftType_Mesa)
    {
        planks = planks->setValue(PlanksBlock::VARIANT, PlanksBlock::Variant::DARK_OAK);
    }
    return planks;
}

const BlockState* MineShaftPieces::MineShaftPiece::getFenceBlock()
{
    if (mType != MineShaftFeature::eMineShaftType_Mesa)
        return Blocks::fence->defaultBlockState();
    return Blocks::dark_oak_fence->defaultBlockState();
}

bool MineShaftPieces::MineShaftPiece::isSupportingBox(Level* level, BoundingBox* box, int x0, int x1, int y, int z)
{
    for (int x = x0; x <= x1; ++x)
    {
        if (getBlock(level, x, y + 1, z, box)->getBlock()->mMaterial == Material::AIR)
            return false;
    }
    return true;
}

void MineShaftPieces::MineShaftPiece::addAdditionalSaveData(DataOutputStream* output)
{
    output->writeInt(static_cast<int>(mType));
}

void MineShaftPieces::MineShaftPiece::readAdditionalSaveData(DataInputStream* input, StructureManager* structureManager)
{
    if (StructureFeatureIO::getVersion() < 2)
        mType = MineShaftFeature::eMineShaftType_Normal;
    else
        mType = static_cast<MineShaftFeature::MineShaftType>(input->readInt());
}

MineShaftPieces::MineShaftCorridor::MineShaftCorridor() : MineShaftPiece()
{
}

MineShaftPieces::MineShaftCorridor::MineShaftCorridor(int genDepth, Random& random, BoundingBox* box,
                                                      const Direction* direction, MineShaftFeature::MineShaftType type)
    : MineShaftPiece(genDepth, type)
{
    setOrientation(direction);
    mBoundingBox = box;
    bool hasRails = random.nextInt(3) == 0;
    mHasPlacedSupport = false;
    mHasRails = hasRails;
    mHasSpiderWebs = !hasRails && random.nextInt(23) == 0;
    if (getOrientation()->getAxis() == Direction::Axis::Z)
    {
        const int length = box->getZSpan();
        mNumSections = length / 5;
    }
    else
    {
        const int length = box->getXSpan();
        mNumSections = length / 5;
    }
}

BoundingBox* MineShaftPieces::MineShaftCorridor::findCorridorSize(std::vector<StructurePiece*>* pieces, Random& random,
                                                                  int x, int y, int z, const Direction* direction)
{
    BoundingBox* box = new BoundingBox(x, y, z, x, y + 2, z);
    int pieceLength = random.nextInt(3) + 2;
    while (pieceLength > 0)
    {
        int blockLength = pieceLength * 5;
        switch (direction->get3DDataValue())
        {
        case 3:
            box->x1 = x + 2;
            box->z1 = z + blockLength - 1;
            break;
        case 4:
            box->x0 = (x + 1) - blockLength;
            box->z1 = z + 2;
            break;
        case 5:
            box->x1 = x + blockLength - 1;
            box->z1 = z + 2;
            break;
        default:
            box->z0 = (z + 1) - blockLength;
            box->x1 = x + 2;
            break;
        }

        if (StructurePiece::findCollisionPiece(pieces, box) == NULL)
            return box;
        --pieceLength;
    }

    delete box;
    return NULL;
}

void MineShaftPieces::MineShaftCorridor::addAdditionalSaveData(DataOutputStream* output)
{
    MineShaftPiece::addAdditionalSaveData(output);
    output->writeBoolean(mHasRails);
    output->writeBoolean(mHasSpiderWebs);
    output->writeBoolean(mHasPlacedSupport);
    output->writeInt(mNumSections);
}

void MineShaftPieces::MineShaftCorridor::readAdditionalSaveData(DataInputStream* input,
                                                                StructureManager* structureManager)
{
    MineShaftPiece::readAdditionalSaveData(input, structureManager);
    mHasRails = input->readBoolean();
    mHasSpiderWebs = input->readBoolean();
    mHasPlacedSupport = input->readBoolean();
    mNumSections = input->readInt();
}

void MineShaftPieces::MineShaftCorridor::addChildren(StructurePiece* startPiece, std::vector<StructurePiece*>* pieces,
                                                     Random& random)
{
    int genDepth = getGenDepth();
    int roll = random.nextInt(4);
    const Direction* dir = getOrientation();
    if (dir != NULL)
    {
        int dataValue = dir->get3DDataValue();
        if (dataValue < 3 || dataValue > 5)
        {
            BoundingBox* box = getBoundingBox();
            int yBase = box->y0 - 1;
            if (roll < 2)
                generateAndAddPiece(startPiece, pieces, random, box->x0, yBase + random.nextInt(3), box->z0 - 1, dir,
                                    genDepth);
            else if (roll == 2)
                generateAndAddPiece(startPiece, pieces, random, box->x0 - 1, yBase + random.nextInt(3), box->z0,
                                    Direction::WEST, genDepth);
            else
                generateAndAddPiece(startPiece, pieces, random, box->x1 + 1, yBase + random.nextInt(3), box->z0,
                                    Direction::EAST, genDepth);
        }
        else if (dataValue == 3)
        {
            BoundingBox* box = getBoundingBox();
            int yBase = box->y0 - 1;
            if (roll < 2)
                generateAndAddPiece(startPiece, pieces, random, box->x0, yBase + random.nextInt(3), box->z1 + 1, dir,
                                    genDepth);
            else if (roll == 2)
                generateAndAddPiece(startPiece, pieces, random, box->x0 - 1, yBase + random.nextInt(3), box->z1 - 3,
                                    Direction::WEST, genDepth);
            else
                generateAndAddPiece(startPiece, pieces, random, box->x1 + 1, yBase + random.nextInt(3), box->z1 - 3,
                                    Direction::EAST, genDepth);
        }
        else
        {
            BoundingBox* box = getBoundingBox();
            int x;
            int y = box->y0 - 1;
            if (dataValue < 5)
            {
                x = box->x0;
                if (roll < 2)
                {
                    generateAndAddPiece(startPiece, pieces, random, x - 1, y + random.nextInt(3), box->z0, dir,
                                        genDepth);
                    goto rooms;
                }
                y += random.nextInt(3);
            }
            else
            {
                x = box->x1;
                if (roll < 2)
                {
                    generateAndAddPiece(startPiece, pieces, random, x + 1, y + random.nextInt(3), box->z0, dir,
                                        genDepth);
                    goto rooms;
                }
                x -= 3;
                y += random.nextInt(3);
            }
            if (roll == 2)
                generateAndAddPiece(startPiece, pieces, random, x, y, box->z0 - 1, Direction::NORTH, genDepth);
            else
                generateAndAddPiece(startPiece, pieces, random, x, y, box->z1 + 1, Direction::SOUTH, genDepth);
        }
    }
rooms:
    if (genDepth < 8)
    {
        BoundingBox* box = getBoundingBox();
        if (dir == Direction::NORTH || dir == Direction::SOUTH)
        {
            int z0 = box->z0 + 3;
            int z1 = box->z0 + 6;
            if (z1 <= box->z1)
            {
                do
                {
                    int pick = random.nextInt(5);
                    if (pick == 0)
                        generateAndAddPiece(startPiece, pieces, random, box->x0 - 1, box->y0, z0, Direction::WEST,
                                            genDepth + 1);
                    else if (pick == 1)
                        generateAndAddPiece(startPiece, pieces, random, box->x1 + 1, box->y0, z0, Direction::EAST,
                                            genDepth + 1);
                    z1 += 5;
                    z0 += 5;
                } while (z1 <= box->z1);
                return;
            }
        }
        else
        {
            int x0 = box->x0 + 3;
            int x1 = box->x0 + 6;
            if (x1 <= box->x1)
            {
                do
                {
                    int pick = random.nextInt(5);
                    if (pick == 0)
                        generateAndAddPiece(startPiece, pieces, random, x0, box->y0, box->z0 - 1, Direction::NORTH,
                                            genDepth + 1);
                    else if (pick == 1)
                        generateAndAddPiece(startPiece, pieces, random, x0, box->y0, box->z1 + 1, Direction::SOUTH,
                                            genDepth + 1);
                    x1 += 5;
                    x0 += 5;
                } while (x1 <= box->x1);
            }
        }
    }
}

bool MineShaftPieces::MineShaftCorridor::createChest(Level* level, BoundingBox* box, Random& random, int x, int y,
                                                     int z, ResourceLocation* lootTable)
{
    const BlockPos& position = BlockPos(getWorldX(x, z), getWorldY(y), getWorldZ(x, z));
    if (static_cast<unsigned int>(level->countInstanceOf(EntityType::MINECART, false, NULL, NULL)) < 100u)
    {
        if (box->isInside(position))
        {
            if (level->getBlockState(position)->getBlock()->mMaterial == Material::AIR)
            {
                if (level->getBlockState(position.below())->getBlock()->mMaterial != Material::AIR)
                {
                    const BlockState* railState = Blocks::rail->defaultBlockState()->setValue(
                        RailBlock::SHAPE, random.nextBoolean() ? BaseRailBlock::RailShape::NORTH_SOUTH
                                                               : BaseRailBlock::RailShape::EAST_WEST);
                    placeBlock(level, railState, x, y, z, box);

                    boost::shared_ptr<MinecartChest> minecart(
                        new MinecartChest(level, (double)((float)position.x + 0.5f), (double)((float)position.y + 0.5f),
                                          (double)((float)position.z + 0.5f)));
                    minecart->setLootTable(lootTable, random.nextLong());
                    level->addEntity(minecart);
                    return true;
                }
            }
        }
    }
    return false;
}

void MineShaftPieces::MineShaftCorridor::placeSupport(Level* level, BoundingBox* box, int x0, int y0, int z, int y1,
                                                      int x1, Random& random)
{
    if (!isSupportingBox(level, box, x0, x1, y1, z))
        return;

    const BlockState* planks = getPlanksBlock();
    const BlockState* fence = getFenceBlock();
    const BlockState* air = Blocks::air->defaultBlockState();
    generateBox(level, box, x0, y0, z, x0, y1 - 1, z, fence, air, false);
    generateBox(level, box, x1, y0, z, x1, y1 - 1, z, fence, air, false);
    if (random.nextInt(4) == 0)
    {
        generateBox(level, box, x0, y1, z, x0, y1, z, planks, air, false);
        generateBox(level, box, x1, y1, z, x1, y1, z, planks, air, false);
        return;
    }

    generateBox(level, box, x0, y1, z, x1, y1, z, planks, air, false);
    maybeGenerateBlock(level, box, random, 0.05f, x0 + 1, y1, z - 1,
                       Blocks::torch->defaultBlockState()->setValue(TorchBlock::FACING, Direction::NORTH));
    maybeGenerateBlock(level, box, random, 0.05f, x0 + 1, y1, z + 1,
                       Blocks::torch->defaultBlockState()->setValue(TorchBlock::FACING, Direction::SOUTH));
}

void MineShaftPieces::MineShaftCorridor::placeCobWeb(Level* level, BoundingBox* box, Random& random, float chance,
                                                     int x, int y, int z)
{
    if (!isAboveGround(level, x, y, z))
        maybeGenerateBlock(level, box, random, chance, x, y, z, Blocks::web->defaultBlockState());
}

bool MineShaftPieces::MineShaftCorridor::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (edgesLiquid(level, box))
        return false;
    int zSpanMax = mNumSections * 5 - 1;
    const BlockState* planks = getPlanksBlock();
    generateBox(level, box, 0, 0, 0, 2, 1, zSpanMax, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateMaybeBox(level, box, random, 0.8f, 0, 2, 0, 2, 2, zSpanMax, Blocks::air->defaultBlockState(),
                     Blocks::air->defaultBlockState(), false, false);
    if (mHasSpiderWebs)
        generateMaybeBox(level, box, random, 0.6f, 0, 0, 0, 2, 1, zSpanMax, Blocks::web->defaultBlockState(),
                         Blocks::air->defaultBlockState(), false, true);

    for (int section = 0; section < mNumSections; ++section)
    {
        int z = section * 5 + 2;
        placeSupport(level, box, 0, 0, z, 2, 2, random);
        placeCobWeb(level, box, random, 0.1f, 0, 2, z - 1);
        placeCobWeb(level, box, random, 0.1f, 2, 2, z - 1);
        placeCobWeb(level, box, random, 0.1f, 0, 2, z + 1);
        placeCobWeb(level, box, random, 0.1f, 2, 2, z + 1);
        placeCobWeb(level, box, random, 0.05f, 0, 2, z - 2);
        placeCobWeb(level, box, random, 0.05f, 2, 2, z - 2);
        placeCobWeb(level, box, random, 0.05f, 0, 2, z + 2);
        placeCobWeb(level, box, random, 0.05f, 2, 2, z + 2);
        if (random.nextInt(100) == 0)
            createChest(level, box, random, 2, 0, z - 1, &BuiltInLootTables::ABANDONED_MINESHAFT);
        if (random.nextInt(100) == 0)
            createChest(level, box, random, 0, 0, z + 1, &BuiltInLootTables::ABANDONED_MINESHAFT);
        if (mHasSpiderWebs && !mHasPlacedSupport)
        {
            int y = getWorldY(0);
            int zPos = z - 1 + random.nextInt(3);
            int wx = getWorldX(1, zPos);
            int wz = getWorldZ(1, zPos);
            const BlockPos& pos = BlockPos(wx, y, wz);
            if (box->isInside(pos) && !isAboveGround(level, 1, 0, zPos))
            {
                setDirty();
                mHasPlacedSupport = true;
                level->setBlock(pos, Blocks::mob_spawner->defaultBlockState(), Block::UPDATE_CLIENTS, false);
                boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
                if (blockEntity != NULL)
                {
                    bool isSpawner = eTYPE_DERIVED_FROM(BlockEntityType::MOB_SPAWNER, blockEntity->GetType());
                    if (isSpawner)
                    {
                        boost::shared_ptr<MobSpawnerBlockEntity> spawner =
                            boost::static_pointer_cast<MobSpawnerBlockEntity>(blockEntity);
                        spawner->getSpawner()->setEntityId(EntityIO::getKey(EntityType::CAVE_SPIDER));
                    }
                }
            }
        }
    }

    for (int x = 0; x <= 2; ++x)
    {
        for (int z = 0; z <= zSpanMax; ++z)
        {
            const BlockState* wallState = getBlock(level, x, -1, z, box);
            if (wallState->getBlock()->mMaterial == Material::AIR && !isAboveGround(level, x, -1, z))
                placeBlock(level, planks, x, -1, z, box);
        }
    }

    if (mHasRails)
    {
        const BlockState* rail = Blocks::rail->defaultBlockState();
        EnumProperty<BaseRailBlock::RailShape*>* shapeProperty = RailBlock::SHAPE;
        BaseRailBlock::RailShape* value = BaseRailBlock::RailShape::NORTH_SOUTH;
        {
            TypedBoxed<BaseRailBlock::RailShape*> shape(value);
            rail = rail->setBoxedValue(shapeProperty, &shape);
        }
        for (int z = 0; z <= zSpanMax; ++z)
        {
            const BlockState* floor = getBlock(level, 1, -1, z, box);
            if (floor->getMaterial() != Material::AIR && floor->isSolid())
            {
                float chance = isAboveGround(level, 1, 0, z) ? 0.7f : 0.9f;
                maybeGenerateBlock(level, box, random, chance, 1, 0, z, rail);
            }
        }
    }

    return true;
}

StructurePiece* MineShaftPieces::MineShaftCorridor::Create()
{
    return new MineShaftCorridor();
}

EStructurePiece MineShaftPieces::MineShaftCorridor::GetType()
{
    return eStructurePiece_MineShaftCorridor;
}

MineShaftPieces::MineShaftCrossing::MineShaftCrossing() : MineShaftPiece()
{
}

MineShaftPieces::MineShaftCrossing::MineShaftCrossing(int genDepth, Random& random, BoundingBox* box,
                                                      const Direction* direction, MineShaftFeature::MineShaftType type)
    : MineShaftPiece(genDepth, type),
      mDirection(direction)
{
    mIsTwoFloored = box->getYSpan() > 3;
    mBoundingBox = box;
}

BoundingBox* MineShaftPieces::MineShaftCrossing::findCrossing(std::vector<StructurePiece*>* pieces, Random& random,
                                                              int x, int y, int z, const Direction* direction)
{
    BoundingBox* box = new BoundingBox(x, y, z, x, y + 2, z);
    if (random.nextInt(4) == 0)
        box->y1 += 4;

    int dir = direction->get3DDataValue();
    if (dir > 2)
    {
        if (dir == 3)
        {
            box->x1 = x + 3;
            box->z1 = z + 4;
            box->x0 = x - 1;
        }
        else if (dir < 5)
        {
            box->z1 = z + 3;
            box->z0 = z - 1;
            box->x0 = x - 4;
        }
        else if (dir == 5)
        {
            box->x1 = x + 4;
            box->z1 = z + 3;
            box->z0 = z - 1;
        }
    }
    else
    {
        box->x1 = x + 3;
        box->z0 = z - 4;
        box->x0 = x - 1;
    }

    if (StructurePiece::findCollisionPiece(pieces, box) != NULL)
    {
        delete box;
        return NULL;
    }
    return box;
}

void MineShaftPieces::MineShaftCrossing::addAdditionalSaveData(DataOutputStream* output)
{
    MineShaftPiece::addAdditionalSaveData(output);
    output->writeBoolean(mIsTwoFloored);
    output->writeInt(mDirection->get2DDataValue());
}

void MineShaftPieces::MineShaftCrossing::readAdditionalSaveData(DataInputStream* input,
                                                                StructureManager* structureManager)
{
    MineShaftPiece::readAdditionalSaveData(input, structureManager);
    mIsTwoFloored = input->readBoolean();
    mDirection = Direction::from2DDataValue(input->readInt());
}

void MineShaftPieces::MineShaftCrossing::addChildren(StructurePiece* startPiece, std::vector<StructurePiece*>* pieces,
                                                     Random& random)
{
    int genDepth = getGenDepth();
    int dir = mDirection->get3DDataValue();
    BoundingBox* box = getBoundingBox();
    bool twoFloored;
    if (dir == 3)
    {
        generateAndAddPiece(startPiece, pieces, random, box->x0 + 1, box->y0, box->z1 + 1, Direction::SOUTH, genDepth);
        generateAndAddPiece(startPiece, pieces, random, box->x0 - 1, box->y0, box->z0 + 1, Direction::WEST, genDepth);
        generateAndAddPiece(startPiece, pieces, random, box->x1 + 1, box->y0, box->z0 + 1, Direction::EAST, genDepth);
        twoFloored = mIsTwoFloored;
    }
    else if (dir == 4)
    {
        generateAndAddPiece(startPiece, pieces, random, box->x0 + 1, box->y0, box->z0 - 1, Direction::NORTH, genDepth);
        generateAndAddPiece(startPiece, pieces, random, box->x0 + 1, box->y0, box->z1 + 1, Direction::SOUTH, genDepth);
        generateAndAddPiece(startPiece, pieces, random, box->x0 - 1, box->y0, box->z0 + 1, Direction::WEST, genDepth);
        twoFloored = mIsTwoFloored;
    }
    else if (dir == 5)
    {
        generateAndAddPiece(startPiece, pieces, random, box->x0 + 1, box->y0, box->z0 - 1, Direction::NORTH, genDepth);
        generateAndAddPiece(startPiece, pieces, random, box->x0 + 1, box->y0, box->z1 + 1, Direction::SOUTH, genDepth);
        generateAndAddPiece(startPiece, pieces, random, box->x1 + 1, box->y0, box->z0 + 1, Direction::EAST, genDepth);
        twoFloored = mIsTwoFloored;
    }
    else
    {
        generateAndAddPiece(startPiece, pieces, random, box->x0 + 1, box->y0, box->z0 - 1, Direction::NORTH, genDepth);
        generateAndAddPiece(startPiece, pieces, random, box->x0 - 1, box->y0, box->z0 + 1, Direction::WEST, genDepth);
        generateAndAddPiece(startPiece, pieces, random, box->x1 + 1, box->y0, box->z0 + 1, Direction::EAST, genDepth);
        twoFloored = mIsTwoFloored;
    }

    if (twoFloored)
    {
        if (random.nextBoolean())
            generateAndAddPiece(startPiece, pieces, random, box->x0 + 1, box->y0 + 4, box->z0 - 1, Direction::NORTH,
                                genDepth);
        if (random.nextBoolean())
            generateAndAddPiece(startPiece, pieces, random, box->x0 - 1, box->y0 + 4, box->z0 + 1, Direction::WEST,
                                genDepth);
        if (random.nextBoolean())
            generateAndAddPiece(startPiece, pieces, random, box->x1 + 1, box->y0 + 4, box->z0 + 1, Direction::EAST,
                                genDepth);
        if (random.nextBoolean())
            generateAndAddPiece(startPiece, pieces, random, box->x0 + 1, box->y0 + 4, box->z1 + 1, Direction::SOUTH,
                                genDepth);
    }
}

void MineShaftPieces::MineShaftCrossing::placeSupportPillar(Level* level, BoundingBox* box, int x, int y0, int z,
                                                            int y1)
{
    if (getBlock(level, x, y1 + 1, z, box)->getBlock()->mMaterial != Material::AIR)
        generateBox(level, box, x, y0, z, x, y1, z, getPlanksBlock(), Blocks::air->defaultBlockState(), false);
}

bool MineShaftPieces::MineShaftCrossing::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (edgesLiquid(level, box))
        return false;

    const BlockState* planks = getPlanksBlock();
    if (mIsTwoFloored)
    {
        generateBox(level, box, mBoundingBox->x0 + 1, mBoundingBox->y0, mBoundingBox->z0, mBoundingBox->x1 - 1,
                    mBoundingBox->y0 + 2, mBoundingBox->z1, Blocks::air->defaultBlockState(),
                    Blocks::air->defaultBlockState(), false);
        generateBox(level, box, mBoundingBox->x0, mBoundingBox->y0, mBoundingBox->z0 + 1, mBoundingBox->x1,
                    mBoundingBox->y0 + 2, mBoundingBox->z1 - 1, Blocks::air->defaultBlockState(),
                    Blocks::air->defaultBlockState(), false);
        generateBox(level, box, mBoundingBox->x0 + 1, mBoundingBox->y1 - 2, mBoundingBox->z0, mBoundingBox->x1 - 1,
                    mBoundingBox->y1, mBoundingBox->z1, Blocks::air->defaultBlockState(),
                    Blocks::air->defaultBlockState(), false);
        generateBox(level, box, mBoundingBox->x0, mBoundingBox->y1 - 2, mBoundingBox->z0 + 1, mBoundingBox->x1,
                    mBoundingBox->y1, mBoundingBox->z1 - 1, Blocks::air->defaultBlockState(),
                    Blocks::air->defaultBlockState(), false);
        generateBox(level, box, mBoundingBox->x0 + 1, mBoundingBox->y0 + 3, mBoundingBox->z0 + 1, mBoundingBox->x1 - 1,
                    mBoundingBox->y0 + 3, mBoundingBox->z1 - 1, Blocks::air->defaultBlockState(),
                    Blocks::air->defaultBlockState(), false);
    }
    else
    {
        generateBox(level, box, mBoundingBox->x0 + 1, mBoundingBox->y0, mBoundingBox->z0, mBoundingBox->x1 - 1,
                    mBoundingBox->y1, mBoundingBox->z1, Blocks::air->defaultBlockState(),
                    Blocks::air->defaultBlockState(), false);
        generateBox(level, box, mBoundingBox->x0, mBoundingBox->y0, mBoundingBox->z0 + 1, mBoundingBox->x1,
                    mBoundingBox->y1, mBoundingBox->z1 - 1, Blocks::air->defaultBlockState(),
                    Blocks::air->defaultBlockState(), false);
    }

    placeSupportPillar(level, box, mBoundingBox->x0 + 1, mBoundingBox->y0, mBoundingBox->z0 + 1, mBoundingBox->y1);
    placeSupportPillar(level, box, mBoundingBox->x0 + 1, mBoundingBox->y0, mBoundingBox->z1 - 1, mBoundingBox->y1);
    placeSupportPillar(level, box, mBoundingBox->x1 - 1, mBoundingBox->y0, mBoundingBox->z0 + 1, mBoundingBox->y1);
    placeSupportPillar(level, box, mBoundingBox->x1 - 1, mBoundingBox->y0, mBoundingBox->z1 - 1, mBoundingBox->y1);

    for (int x = mBoundingBox->x0; x <= mBoundingBox->x1; ++x)
    {
        for (int z = mBoundingBox->z0; z <= mBoundingBox->z1; ++z)
        {
            if (getBlock(level, x, mBoundingBox->y0 - 1, z, box)->getBlock()->mMaterial == Material::AIR
                && !isAboveGround(level, x, mBoundingBox->y0 - 1, z))
                placeBlock(level, planks, x, mBoundingBox->y0 - 1, z, box);
        }
    }
    return true;
}

StructurePiece* MineShaftPieces::MineShaftCrossing::Create()
{
    return new MineShaftCrossing();
}

EStructurePiece MineShaftPieces::MineShaftCrossing::GetType()
{
    return eStructurePiece_MineShaftCrossing;
}

MineShaftPieces::MineShaftRoom::MineShaftRoom(int genDepth, Random& random, int x, int z,
                                              MineShaftFeature::MineShaftType type)
    : MineShaftPiece(genDepth, type)
{
    mBoundingBox = new BoundingBox(x, 50, z, x + 7 + random.nextInt(6), 54 + random.nextInt(6),
                                   z + 7 + random.nextInt(6));
}

MineShaftPieces::MineShaftRoom::MineShaftRoom() : MineShaftPiece()
{
}

MineShaftPieces::MineShaftRoom::~MineShaftRoom()
{
    for (std::list<BoundingBox*>::iterator it = mChildPieces.begin(); it != mChildPieces.end(); ++it)
        delete *it;
}

void MineShaftPieces::MineShaftRoom::addChildren(StructurePiece* startPiece, std::vector<StructurePiece*>* pieces,
                                                 Random& random)
{
    int genDepth = getGenDepth();
    BoundingBox* box = getBoundingBox();
    int yRange = box->getYSpan() - 4;
    if (yRange < 1)
        yRange = 1;

    int xAccum = 0;
    while (box->getXSpan() > 0)
    {
        int xRand = random.nextInt(box->getXSpan());
        xAccum += xRand;
        if (box->getXSpan() < xAccum + 3)
            break;
        int yRand = random.nextInt(yRange);
        StructurePiece* piece = generateAndAddPiece(startPiece, pieces, random, box->x0 + xAccum, box->y0 + yRand + 1,
                                                    box->z0 - 1, Direction::NORTH, genDepth);
        if (piece != NULL)
        {
            BoundingBox* childBox = piece->getBoundingBox();
            mChildPieces.push_back(
                new BoundingBox(childBox->x0, childBox->y0, box->z0, childBox->x1, childBox->y1, box->z0 - 1));
        }
        xAccum += 4;
    }

    xAccum = 0;
    while (box->getXSpan() > 0)
    {
        int xRand = random.nextInt(box->getXSpan());
        xAccum += xRand;
        if (box->getXSpan() < xAccum + 3)
            break;
        int yRand = random.nextInt(yRange);
        generateAndAddPiece(startPiece, pieces, random, box->x0 + xAccum, box->y0 + yRand + 1, box->z1 + 1,
                            Direction::SOUTH, genDepth);
        xAccum += 4;
    }

    int zAccum = 0;
    while (box->getZSpan() > 0)
    {
        int zRand = random.nextInt(box->getZSpan());
        zAccum += zRand;
        if (box->getZSpan() < zAccum + 3)
            break;
        int yRand = random.nextInt(yRange);
        StructurePiece* piece = generateAndAddPiece(startPiece, pieces, random, box->x0 - 1, box->y0 + yRand + 1,
                                                    box->z0 + zAccum, Direction::WEST, genDepth);
        if (piece != NULL)
        {
            BoundingBox* childBox = piece->getBoundingBox();
            mChildPieces.push_back(
                new BoundingBox(box->x0, childBox->y0, childBox->z0, box->x0 + 1, childBox->y1, childBox->z1));
        }
        zAccum += 4;
    }

    zAccum = 0;
    while (box->getZSpan() > 0)
    {
        int zRand = random.nextInt(box->getZSpan());
        zAccum += zRand;
        if (box->getZSpan() < zAccum + 3)
            return;
        int yRand = random.nextInt(yRange);
        StructurePiece* piece = generateAndAddPiece(startPiece, pieces, random, box->x1 + 1, box->y0 + yRand + 1,
                                                    box->z0 + zAccum, Direction::EAST, genDepth);
        if (piece != NULL)
        {
            BoundingBox* childBox = piece->getBoundingBox();
            mChildPieces.push_back(
                new BoundingBox(box->x1 - 1, childBox->y0, childBox->z0, box->x1, childBox->y1, childBox->z1));
        }
        zAccum += 4;
    }
}

void MineShaftPieces::MineShaftRoom::move(int x, int y, int z)
{
    StructurePiece::move(x, y, z);
    for (std::list<BoundingBox*>::iterator it = mChildPieces.begin(); it != mChildPieces.end(); ++it)
        (*it)->move(x, y, z);
}

bool MineShaftPieces::MineShaftRoom::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (edgesLiquid(level, box))
        return false;

    generateBox(level, box, mBoundingBox->x0, mBoundingBox->y0, mBoundingBox->z0, mBoundingBox->x1, mBoundingBox->y0,
                mBoundingBox->z1, Blocks::dirt->defaultBlockState(), Blocks::air->defaultBlockState(), true);
    int floor = mBoundingBox->y0;
    int roof = floor + 3;
    generateBox(level, box, mBoundingBox->x0, floor + 1, mBoundingBox->z0, mBoundingBox->x1,
                mBoundingBox->y1 < roof ? mBoundingBox->y1 : roof, mBoundingBox->z1, Blocks::air->defaultBlockState(),
                Blocks::air->defaultBlockState(), false);
    for (std::list<BoundingBox*>::iterator it = mChildPieces.begin(); it != mChildPieces.end(); ++it)
    {
        BoundingBox* child = *it;
        generateBox(level, box, child->x0, child->y1 - 2, child->z0, child->x1, child->y1, child->z1,
                    Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(), false);
    }
    generateUpperHalfSphere(level, box, mBoundingBox->x0, mBoundingBox->y0 + 4, mBoundingBox->z0, mBoundingBox->x1,
                            mBoundingBox->y1, mBoundingBox->z1, Blocks::air->defaultBlockState(), false);
    return true;
}

void MineShaftPieces::MineShaftRoom::addAdditionalSaveData(DataOutputStream* output)
{
    MineShaftPiece::addAdditionalSaveData(output);
    output->writeInt(static_cast<int>(mChildPieces.size()));
    for (std::list<BoundingBox*>::const_iterator it = mChildPieces.begin(); it != mChildPieces.end(); ++it)
        (*it)->write(output);
}

void MineShaftPieces::MineShaftRoom::readAdditionalSaveData(DataInputStream* input, StructureManager* structureManager)
{
    MineShaftPiece::readAdditionalSaveData(input, structureManager);
    int count = input->readInt();
    for (int i = 0; i < count; ++i)
    {
        BoundingBox* box = new BoundingBox();
        box->read(input);
        mChildPieces.push_back(box);
    }
}

StructurePiece* MineShaftPieces::MineShaftRoom::Create()
{
    return new MineShaftRoom();
}

EStructurePiece MineShaftPieces::MineShaftRoom::GetType()
{
    return eStructurePiece_MineShaftRoom;
}

MineShaftPieces::MineShaftStairs::MineShaftStairs() : MineShaftPiece()
{
}

MineShaftPieces::MineShaftStairs::MineShaftStairs(int genDepth, Random& random, BoundingBox* box,
                                                  const Direction* direction, MineShaftFeature::MineShaftType type)
    : MineShaftPiece(genDepth, type)
{
    setOrientation(direction);
    mBoundingBox = box;
}

BoundingBox* MineShaftPieces::MineShaftStairs::findStairs(std::vector<StructurePiece*>* pieces, Random& random, int x,
                                                          int y, int z, const Direction* direction)
{
    BoundingBox* box = new BoundingBox(x, y - 5, z, x, y + 2, z);
    int dir = direction->get3DDataValue();
    if (dir > 2)
    {
        if (dir == 3)
        {
            box->x1 = x + 2;
            box->z1 = z + 8;
        }
        else if (dir < 5)
        {
            box->x0 = x - 8;
            box->z1 = z + 2;
        }
        else if (dir == 5)
        {
            box->x1 = x + 8;
            box->z1 = z + 2;
        }
    }
    else
    {
        box->x1 = x + 2;
        box->z0 = z - 8;
    }

    if (StructurePiece::findCollisionPiece(pieces, box) != NULL)
    {
        delete box;
        return NULL;
    }
    return box;
}

void MineShaftPieces::MineShaftStairs::addChildren(StructurePiece* startPiece, std::vector<StructurePiece*>* pieces,
                                                   Random& random)
{
    int genDepth = getGenDepth();
    int dir = getOrientation()->get3DDataValue();
    BoundingBox* box = getBoundingBox();
    if (dir == 3)
    {
        generateAndAddPiece(startPiece, pieces, random, box->x0, box->y0, box->z1 + 1, Direction::SOUTH, genDepth);
        return;
    }
    if (dir > 2 && dir < 5)
    {
        generateAndAddPiece(startPiece, pieces, random, box->x0 - 1, box->y0, box->z0, Direction::WEST, genDepth);
        return;
    }
    if (dir == 5)
    {
        generateAndAddPiece(startPiece, pieces, random, box->x1 + 1, box->y0, box->z0, Direction::EAST, genDepth);
        return;
    }
    generateAndAddPiece(startPiece, pieces, random, box->x0, box->y0, box->z0 - 1, Direction::NORTH, genDepth);
}

bool MineShaftPieces::MineShaftStairs::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (edgesLiquid(level, box))
        return false;
    generateBox(level, box, 0, 5, 0, 2, 7, 1, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 0, 0, 7, 2, 2, 8, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 0, 4, 2, 2, 7, 2, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 0, 3, 3, 2, 6, 3, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 0, 2, 4, 2, 5, 4, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 0, 1, 5, 2, 4, 5, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    generateBox(level, box, 0, 1, 6, 2, 3, 6, Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(),
                false);
    return true;
}

StructurePiece* MineShaftPieces::MineShaftStairs::Create()
{
    return new MineShaftStairs();
}

EStructurePiece MineShaftPieces::MineShaftStairs::GetType()
{
    return eStructurePiece_MineShaftStairs;
}

void MineShaftPieces::loadStatic()
{
    StructureFeatureIO::setPieceId(eStructurePiece_MineShaftCorridor, &MineShaftCorridor::Create, L"MSCorridor");
    StructureFeatureIO::setPieceId(eStructurePiece_MineShaftCrossing, &MineShaftCrossing::Create, L"MSCrossing");
    StructureFeatureIO::setPieceId(eStructurePiece_MineShaftRoom, &MineShaftRoom::Create, L"MSRoom");
    StructureFeatureIO::setPieceId(eStructurePiece_MineShaftStairs, &MineShaftStairs::Create, L"MSStairs");
}

StructurePiece* MineShaftPieces::createRandomShaftPiece(std::vector<StructurePiece*>* pieces, Random& random, int x,
                                                        int y, int z, const Direction* direction, int genDepth,
                                                        MineShaftFeature::MineShaftType type)
{
    int choice = random.nextInt(100);
    BoundingBox* box = NULL;
    if (choice < 70)
    {
        box = MineShaftCorridor::findCorridorSize(pieces, random, x, y, z, direction);
        return box == NULL ? NULL : new MineShaftCorridor(genDepth, random, box, direction, type);
    }
    if (choice < 80)
    {
        box = MineShaftStairs::findStairs(pieces, random, x, y, z, direction);
        return box == NULL ? NULL : new MineShaftStairs(genDepth, random, box, direction, type);
    }
    box = MineShaftCrossing::findCrossing(pieces, random, x, y, z, direction);
    return box == NULL ? NULL : new MineShaftCrossing(genDepth, random, box, direction, type);
}

StructurePiece* MineShaftPieces::generateAndAddPiece(StructurePiece* startPiece, std::vector<StructurePiece*>* pieces,
                                                     Random& random, int x, int y, int z, const Direction* direction,
                                                     int genDepth)
{
    if (startPiece == NULL || pieces == NULL || genDepth >= 9)
        return NULL;
    BoundingBox* parent = startPiece->getBoundingBox();
    if (parent == NULL || x - parent->x0 >= 81 || z - parent->z0 >= 81)
        return NULL;
    MineShaftPiece* shaft = dynamic_cast<MineShaftPiece*>(startPiece);
    if (shaft == NULL)
        return NULL;
    StructurePiece* piece = createRandomShaftPiece(pieces, random, x, y, z, direction, genDepth + 1, shaft->mType);
    if (piece == NULL)
        return NULL;
    pieces->push_back(piece);
    piece->addChildren(startPiece, pieces, random);
    return piece;
}

#include "game/world/generation/ScatteredFeaturePieces.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/BoundingBox.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Direction.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/OceanRuin.hpp"
#include "game/core/Witch.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelType.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/ChestBlockEntity.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/FlowerPotBlock.hpp"
#include "game/world/block/FlowerPotBlockEntity.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/LeverBlock.hpp"
#include "game/world/block/PistonBaseBlock.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/SandStoneBlock.hpp"
#include "game/world/block/StairBlock.hpp"
#include "game/world/block/StoneBrickBlock.hpp"
#include "game/world/block/StoneSlabBlock.hpp"
#include "game/world/block/TripWireBlock.hpp"
#include "game/world/block/TripWireHookBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/VineBlock.hpp"
#include "game/world/generation/FlatGeneratorInfo.hpp"
#include "game/world/generation/StructureFeature.hpp"
#include "game/world/generation/StructureFeatureIO.hpp"
#include "game/world/generation/StructureManager.hpp"
#include "game/world/generation/StructureTemplate.hpp"

ScatteredFeaturePieces::JunglePyramidPiece::MossStoneSelector*
    ScatteredFeaturePieces::JunglePyramidPiece::mStoneSelector = NULL;

void ScatteredFeaturePieces::loadStatic()
{
    StructureFeatureIO::setPieceId(eStructurePiece_DesertPyramidPiece,
                                   &ScatteredFeaturePieces::DesertPyramidPiece::Create, L"TeDP");
    StructureFeatureIO::setPieceId(eStructurePiece_JunglePyramidPiece,
                                   &ScatteredFeaturePieces::JunglePyramidPiece::Create, L"TeJP");
    StructureFeatureIO::setPieceId(eStructurePiece_SwamplandHut, &ScatteredFeaturePieces::SwamplandHut::Create,
                                   L"TeSH");
    StructureFeatureIO::setPieceId(eStructurePiece_Igloo, &ScatteredFeaturePieces::Igloo::Create, L"Iglu");
    StructureFeatureIO::setPieceId(eStructurePiece_OceanRuin, &OceanRuin::Create, L"OcRu");
}

ScatteredFeaturePieces::ScatteredFeaturePiece::ScatteredFeaturePiece() : StructurePiece(), mPlaceSettings()
{
    mWidth = 0;
    mHeight = 0;
    mDepth = 0;
    mHeightPosition = 0;
    mFirstBools.clear();
    mSecondBools.clear();
    mFirstInts.clear();
    mSecondInts.clear();
}

ScatteredFeaturePieces::ScatteredFeaturePiece::ScatteredFeaturePiece(Level* level, Random& random, int x, int y, int z,
                                                                     int width, int height, int depth)
    : StructurePiece(0),
      mPlaceSettings()
{
    mWidth = width;
    mHeight = height;
    mDepth = depth;
    mHeightPosition = -1;
    mFirstBools.clear();
    mSecondBools.clear();
    mFirstInts.clear();
    mSecondInts.clear();

    setOrientation(Direction::Plane::HORIZONTAL->getRandomDirection(random));

    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
    if (options != NULL)
    {
        const Direction* direction = NULL;
        if (options->isFeatureChunk(x >> 4, z >> 4, StructureFeature::eFeatureType_ScatteredFeature, &direction))
            setOrientation(direction);
    }

    if (level->getLevelData()->getGeneratorType() == LevelType::FLAT)
    {
        SuperflatConfig* config = level->getLevelData()->getGeneratorOptions();
        if (config != NULL)
        {
            FlatGeneratorInfo info(config, false);
            y = info.getHeight();
        }
    }

    switch (getOrientation()->getOrdinal())
    {
    case 2:
        setBoundingBox(new BoundingBox(x, y, z - depth, x + width - 1, y + height - 1, z));
        break;
    case 4:
    case 5:
        setBoundingBox(new BoundingBox(x - width, y, z - depth, x, y + height - 1, z));
        break;
    default:
        setOrientation(Direction::SOUTH);
        setBoundingBox(new BoundingBox(x, y, z, x + width - 1, y + height - 1, z + depth - 1));
        break;
    }
}

ScatteredFeaturePieces::ScatteredFeaturePiece::~ScatteredFeaturePiece()
{
}

void ScatteredFeaturePieces::ScatteredFeaturePiece::addAdditionalSaveData(DataOutputStream* output)
{
    output->writeInt(mWidth);
    output->writeInt(mHeight);
    output->writeInt(mDepth);
    output->writeInt(mHeightPosition);
    mPlaceSettings.write(output);
}

void ScatteredFeaturePieces::ScatteredFeaturePiece::readAdditionalSaveData(DataInputStream* input,
                                                                           StructureManager* structureManager)
{
    mWidth = input->readInt();
    mHeight = input->readInt();
    mDepth = input->readInt();
    mHeightPosition = input->readInt();
    if (StructureFeatureIO::getVersion() == 1)
    {
        readStructurePieceSaveData(input, mFirstBools, mSecondBools, mFirstInts, mSecondInts);
        return;
    }
    if (StructureFeatureIO::getVersion() < 3)
        return;
    mPlaceSettings.read(input);
}

bool ScatteredFeaturePieces::ScatteredFeaturePiece::updateAverageGroundHeight(Level* level, BoundingBox* box,
                                                                              int offset, bool useSpawnHeight)
{
    if (mHeightPosition >= 0)
        return true;

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
                if (useSpawnHeight)
                {
                    int height = level->getTopSolidBlockPos(pos).y;
                    int spawnHeight = level->mDimension->getSpawnYPosition();
                    if (height < spawnHeight)
                        height = spawnHeight;
                    ++count;
                    total += height;
                }
                else
                {
                    ++count;
                    total += level->getTopSolidBlockPos(pos).y;
                }
            }
        }
    }

    if (count == 0)
        return false;

    BoundingBox* bounds = mBoundingBox;
    mHeightPosition = total / count;
    bounds->move(0, mHeightPosition - bounds->y0 + offset, 0);
    return true;
}

ScatteredFeaturePieces::DesertPyramidPiece::DesertPyramidPiece() : ScatteredFeaturePiece()
{
    mHasPlacedChest[0] = false;
    mHasPlacedChest[1] = false;
    mHasPlacedChest[2] = false;
    mHasPlacedChest[3] = false;
}

ScatteredFeaturePieces::DesertPyramidPiece::DesertPyramidPiece(Level* level, Random& random, int x, int z)
    : ScatteredFeaturePiece(level, random, x, 64, z, 21, 15, 21)
{
    mHasPlacedChest[0] = false;
    mHasPlacedChest[1] = false;
    mHasPlacedChest[2] = false;
    mHasPlacedChest[3] = false;
}

void ScatteredFeaturePieces::DesertPyramidPiece::addAdditionalSaveData(DataOutputStream* output)
{
    ScatteredFeaturePiece::addAdditionalSaveData(output);
    output->writeBoolean(mHasPlacedChest[0]);
    output->writeBoolean(mHasPlacedChest[1]);
    output->writeBoolean(mHasPlacedChest[2]);
    output->writeBoolean(mHasPlacedChest[3]);
}

void ScatteredFeaturePieces::DesertPyramidPiece::readAdditionalSaveData(DataInputStream* input,
                                                                        StructureManager* structureManager)
{
    ScatteredFeaturePiece::readAdditionalSaveData(input, structureManager);
    if (StructureFeatureIO::getVersion() == 1)
    {
        mHasPlacedChest[0] = mSecondBools.size() > 3 && mSecondBools[0];
        mHasPlacedChest[1] = mSecondBools.size() > 3 && mSecondBools[1];
        mHasPlacedChest[2] = mSecondBools.size() > 3 && mSecondBools[2];
        mHasPlacedChest[3] = mSecondBools.size() > 3 && mSecondBools[3];
    }
    else
    {
        mHasPlacedChest[0] = input->readBoolean();
        mHasPlacedChest[1] = input->readBoolean();
        mHasPlacedChest[2] = input->readBoolean();
        mHasPlacedChest[3] = input->readBoolean();
    }
}

bool ScatteredFeaturePieces::DesertPyramidPiece::postProcess(Level* level, Random& random, BoundingBox* box)
{
    int y = 0;
    while (getWorldY(y) < 14)
        ++y;

    generateBox(level, box, 0, y - 4, 0, mWidth - 1, y, mDepth - 1, Blocks::sandstone->defaultBlockState(),
                Blocks::sandstone->defaultBlockState(), false);
    for (int layer = 1; layer <= 9; ++layer)
    {
        generateBox(level, box, layer, layer + y, layer, mWidth - 1 - layer, layer + y, mDepth - 1 - layer,
                    Blocks::sandstone->defaultBlockState(), Blocks::sandstone->defaultBlockState(), false);
        generateBox(level, box, layer + 1, layer + y, layer + 1, mWidth - 2 - layer, layer + y, mDepth - 2 - layer,
                    Blocks::air->defaultBlockState(), Blocks::air->defaultBlockState(), false);
    }
    fillBoxDown(level, Blocks::sandstone->defaultBlockState(), 0, 0, mWidth, mDepth, y - 5, box);

    const BlockState* stairsNorth = Blocks::sandstone_stairs->defaultBlockState()->setValue(StairBlock::FACING,
                                                                                            Direction::NORTH);
    const BlockState* stairsSouth = Blocks::sandstone_stairs->defaultBlockState()->setValue(StairBlock::FACING,
                                                                                            Direction::SOUTH);
    const BlockState* stairsEast = Blocks::sandstone_stairs->defaultBlockState()->setValue(StairBlock::FACING,
                                                                                           Direction::EAST);
    const BlockState* stairsWest = Blocks::sandstone_stairs->defaultBlockState()->setValue(StairBlock::FACING,
                                                                                           Direction::WEST);
    int orange = ~DyeColor::ORANGE->getItemData() & 0xf;
    int blue = ~DyeColor::BLUE->getItemData() & 0xf;

    generateBox(level, box, 0, y, 0, 4, y + 9, 4, Blocks::sandstone->defaultBlockState(),
                Blocks::air->defaultBlockState(), false);
    generateBox(level, box, 1, y + 10, 1, 3, y + 10, 3, Blocks::sandstone->defaultBlockState(),
                Blocks::sandstone->defaultBlockState(), false);
    placeBlock(level, stairsNorth, 2, y + 10, 0, box);
    placeBlock(level, stairsSouth, 2, y + 10, 4, box);
    placeBlock(level, stairsEast, 0, y + 10, 2, box);
    placeBlock(level, stairsWest, 4, y + 10, 2, box);
    generateBox(level, box, mWidth - 5, y, 0, mWidth - 1, y + 9, 4, Blocks::sandstone->defaultBlockState(),
                Blocks::air->defaultBlockState(), false);
    generateBox(level, box, mWidth - 4, y + 10, 1, mWidth - 2, y + 10, 3, Blocks::sandstone->defaultBlockState(),
                Blocks::sandstone->defaultBlockState(), false);
    placeBlock(level, stairsNorth, mWidth - 3, y + 10, 0, box);
    placeBlock(level, stairsSouth, mWidth - 3, y + 10, 4, box);
    placeBlock(level, stairsEast, mWidth - 5, y + 10, 2, box);
    placeBlock(level, stairsWest, mWidth - 1, y + 10, 2, box);
    generateBox(level, box, 8, y, 0, 12, y + 4, 4, Blocks::sandstone->defaultBlockState(),
                Blocks::air->defaultBlockState(), false);
    generateBox(level, box, 9, y + 1, 0, 11, y + 3, 4, Blocks::air->defaultBlockState(),
                Blocks::air->defaultBlockState(), false);
    placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), 9, y + 1, 1, box);
    placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), 9, y + 2, 1, box);
    placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), 9, y + 3, 1, box);
    placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), 10, y + 3, 1, box);
    placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), 11, y + 3, 1, box);
    placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), 11, y + 2, 1, box);
    placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), 11, y + 1, 1, box);
    generateBox(level, box, 4, y + 1, 1, 8, y + 3, 3, Blocks::sandstone->defaultBlockState(),
                Blocks::air->defaultBlockState(), false);
    generateBox(level, box, 4, y + 1, 2, 8, y + 2, 2, Blocks::air->defaultBlockState(),
                Blocks::air->defaultBlockState(), false);
    generateBox(level, box, 12, y + 1, 1, 16, y + 3, 3, Blocks::sandstone->defaultBlockState(),
                Blocks::air->defaultBlockState(), false);
    generateBox(level, box, 12, y + 1, 2, 16, y + 2, 2, Blocks::air->defaultBlockState(),
                Blocks::air->defaultBlockState(), false);
    generateBox(level, box, 5, y + 4, 5, mWidth - 6, y + 4, mDepth - 6, Blocks::sandstone->defaultBlockState(),
                Blocks::sandstone->defaultBlockState(), false);
    generateBox(level, box, 9, y + 4, 9, 11, y + 4, 11, Blocks::air->defaultBlockState(),
                Blocks::air->defaultBlockState(), false);
    generateBox(level, box, 8, y + 1, 8, 8, y + 3, 8,
                Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()),
                Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), false);
    generateBox(level, box, 12, y + 1, 8, 12, y + 3, 8,
                Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()),
                Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), false);
    generateBox(level, box, 8, y + 1, 12, 8, y + 3, 12,
                Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()),
                Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), false);
    generateBox(level, box, 12, y + 1, 12, 12, y + 3, 12,
                Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()),
                Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), false);
    generateBox(level, box, 1, y + 1, 5, 4, y + 4, 11, Blocks::sandstone->defaultBlockState(),
                Blocks::sandstone->defaultBlockState(), false);
    generateBox(level, box, mWidth - 5, y + 1, 5, mWidth - 2, y + 4, 11, Blocks::sandstone->defaultBlockState(),
                Blocks::sandstone->defaultBlockState(), false);
    generateBox(level, box, 6, y + 7, 9, 6, y + 7, 11, Blocks::sandstone->defaultBlockState(),
                Blocks::sandstone->defaultBlockState(), false);
    generateBox(level, box, mWidth - 7, y + 7, 9, mWidth - 7, y + 7, 11, Blocks::sandstone->defaultBlockState(),
                Blocks::sandstone->defaultBlockState(), false);
    generateBox(level, box, 5, y + 5, 9, 5, y + 7, 11,
                Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()),
                Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), false);
    generateBox(level, box, mWidth - 6, y + 5, 9, mWidth - 6, y + 7, 11,
                Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()),
                Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), false);
    placeBlock(level, Blocks::air->defaultBlockState(), 5, y + 5, 10, box);
    placeBlock(level, Blocks::air->defaultBlockState(), 5, y + 6, 10, box);
    placeBlock(level, Blocks::air->defaultBlockState(), 6, y + 6, 10, box);
    placeBlock(level, Blocks::air->defaultBlockState(), mWidth - 6, y + 5, 10, box);
    placeBlock(level, Blocks::air->defaultBlockState(), mWidth - 6, y + 6, 10, box);
    placeBlock(level, Blocks::air->defaultBlockState(), mWidth - 7, y + 6, 10, box);
    generateBox(level, box, 2, y + 4, 4, 2, y + 6, 4, Blocks::air->defaultBlockState(),
                Blocks::air->defaultBlockState(), false);
    generateBox(level, box, mWidth - 3, y + 4, 4, mWidth - 3, y + 6, 4, Blocks::air->defaultBlockState(),
                Blocks::air->defaultBlockState(), false);
    placeBlock(level, stairsNorth, 2, y + 4, 5, box);
    placeBlock(level, stairsNorth, 2, y + 3, 4, box);
    placeBlock(level, stairsNorth, mWidth - 3, y + 4, 5, box);
    placeBlock(level, stairsNorth, mWidth - 3, y + 3, 4, box);
    generateBox(level, box, 1, y + 1, 3, 2, y + 2, 3, Blocks::sandstone->defaultBlockState(),
                Blocks::sandstone->defaultBlockState(), false);
    generateBox(level, box, mWidth - 3, y + 1, 3, mWidth - 2, y + 2, 3, Blocks::sandstone->defaultBlockState(),
                Blocks::sandstone->defaultBlockState(), false);
    placeBlock(level, Blocks::sandstone->defaultBlockState(), 1, y + 1, 2, box);
    placeBlock(level, Blocks::sandstone->defaultBlockState(), mWidth - 2, y + 1, 2, box);
    placeBlock(level, Blocks::stone_slab->getBlockState(StoneSlabBlock::Variant::SAND->getData()), 1, y + 2, 2, box);
    placeBlock(level, Blocks::stone_slab->getBlockState(StoneSlabBlock::Variant::SAND->getData()), mWidth - 2, y + 2, 2,
               box);
    placeBlock(level, stairsWest, 2, y + 1, 2, box);
    placeBlock(level, stairsEast, mWidth - 3, y + 1, 2, box);
    generateBox(level, box, 4, y + 3, 5, 4, y + 3, 18, Blocks::sandstone->defaultBlockState(),
                Blocks::sandstone->defaultBlockState(), false);
    generateBox(level, box, mWidth - 5, y + 3, 5, mWidth - 5, y + 3, 17, Blocks::sandstone->defaultBlockState(),
                Blocks::sandstone->defaultBlockState(), false);
    generateBox(level, box, 3, y + 1, 5, 4, y + 2, 16, Blocks::air->defaultBlockState(),
                Blocks::air->defaultBlockState(), false);
    generateBox(level, box, mWidth - 6, y + 1, 5, mWidth - 5, y + 2, 16, Blocks::air->defaultBlockState(),
                Blocks::air->defaultBlockState(), false);

    for (int z = 5; z <= 17; z += 2)
    {
        placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), 4, y + 1, z, box);
        placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::CHISELED->getData()), 4, y + 2, z, box);
        placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), mWidth - 5, y + 1, z,
                   box);
        placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::CHISELED->getData()), mWidth - 5, y + 2, z,
                   box);
    }

    placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), 10, y, 7, box);
    placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), 10, y, 8, box);
    placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), 9, y, 9, box);
    placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), 11, y, 9, box);
    placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), 8, y, 10, box);
    placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), 12, y, 10, box);
    placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), 7, y, 10, box);
    placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), 13, y, 10, box);
    placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), 9, y, 11, box);
    placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), 11, y, 11, box);
    placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), 10, y, 12, box);
    placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), 10, y, 13, box);
    placeBlock(level, Blocks::stained_hardened_clay->getBlockState(blue), 10, y, 10, box);

    for (int x = 0; x <= mWidth - 1; x += mWidth - 1)
    {
        placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), x, y + 2, 1, box);
        placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), x, y + 2, 2, box);
        placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), x, y + 2, 3, box);
        placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), x, y + 3, 1, box);
        placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), x, y + 3, 2, box);
        placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), x, y + 3, 3, box);
        placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), x, y + 4, 1, box);
        placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::CHISELED->getData()), x, y + 4, 2, box);
        placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), x, y + 4, 3, box);
        placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), x, y + 5, 1, box);
        placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), x, y + 5, 2, box);
        placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), x, y + 5, 3, box);
        placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), x, y + 6, 1, box);
        placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::CHISELED->getData()), x, y + 6, 2, box);
        placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), x, y + 6, 3, box);
        placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), x, y + 7, 1, box);
        placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), x, y + 7, 2, box);
        placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), x, y + 7, 3, box);
        placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), x, y + 8, 1, box);
        placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), x, y + 8, 2, box);
        placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), x, y + 8, 3, box);
    }

    for (int x = 2; x <= mWidth - 3; x += mWidth - 3 - 2)
    {
        placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), x - 1, y + 2, 0, box);
        placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), x, y + 2, 0, box);
        placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), x + 1, y + 2, 0, box);
        placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), x - 1, y + 3, 0, box);
        placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), x, y + 3, 0, box);
        placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), x + 1, y + 3, 0, box);
        placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), x - 1, y + 4, 0, box);
        placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::CHISELED->getData()), x, y + 4, 0, box);
        placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), x + 1, y + 4, 0, box);
        placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), x - 1, y + 5, 0, box);
        placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), x, y + 5, 0, box);
        placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), x + 1, y + 5, 0, box);
        placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), x - 1, y + 6, 0, box);
        placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::CHISELED->getData()), x, y + 6, 0, box);
        placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), x + 1, y + 6, 0, box);
        placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), x - 1, y + 7, 0, box);
        placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), x, y + 7, 0, box);
        placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), x + 1, y + 7, 0, box);
        placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), x - 1, y + 8, 0, box);
        placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), x, y + 8, 0, box);
        placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), x + 1, y + 8, 0, box);
    }

    generateBox(level, box, 8, y + 4, 0, 12, y + 6, 0,
                Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()),
                Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), false);
    placeBlock(level, Blocks::air->defaultBlockState(), 8, y + 6, 0, box);
    placeBlock(level, Blocks::air->defaultBlockState(), 12, y + 6, 0, box);
    placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), 9, y + 5, 0, box);
    placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::CHISELED->getData()), 10, y + 5, 0, box);
    placeBlock(level, Blocks::stained_hardened_clay->getBlockState(orange), 11, y + 5, 0, box);
    generateBox(level, box, 8, y - 14, 8, 12, y - 11, 12,
                Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()),
                Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), false);
    generateBox(level, box, 8, y - 10, 8, 12, y - 10, 12,
                Blocks::sandstone->getBlockState(SandStoneBlock::CHISELED->getData()),
                Blocks::sandstone->getBlockState(SandStoneBlock::CHISELED->getData()), false);
    generateBox(level, box, 8, y - 9, 8, 12, y - 9, 12,
                Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()),
                Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), false);
    generateBox(level, box, 8, y - 8, 8, 12, y - 1, 12, Blocks::sandstone->defaultBlockState(),
                Blocks::sandstone->defaultBlockState(), false);
    generateBox(level, box, 9, y - 11, 9, 11, y - 1, 11, Blocks::air->defaultBlockState(),
                Blocks::air->defaultBlockState(), false);
    placeBlock(level, Blocks::stone_pressure_plate->defaultBlockState(), 10, y - 11, 10, box);
    generateBox(level, box, 9, y - 13, 9, 11, y - 13, 11, Blocks::tnt->defaultBlockState(),
                Blocks::air->defaultBlockState(), false);
    placeBlock(level, Blocks::air->defaultBlockState(), 8, y - 11, 10, box);
    placeBlock(level, Blocks::air->defaultBlockState(), 8, y - 10, 10, box);
    placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::CHISELED->getData()), 7, y - 10, 10, box);
    placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), 7, y - 11, 10, box);
    placeBlock(level, Blocks::air->defaultBlockState(), 12, y - 11, 10, box);
    placeBlock(level, Blocks::air->defaultBlockState(), 12, y - 10, 10, box);
    placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::CHISELED->getData()), 13, y - 10, 10, box);
    placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), 13, y - 11, 10, box);
    placeBlock(level, Blocks::air->defaultBlockState(), 10, y - 11, 8, box);
    placeBlock(level, Blocks::air->defaultBlockState(), 10, y - 10, 8, box);
    placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::CHISELED->getData()), 10, y - 10, 7, box);
    placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), 10, y - 11, 7, box);
    placeBlock(level, Blocks::air->defaultBlockState(), 10, y - 11, 12, box);
    placeBlock(level, Blocks::air->defaultBlockState(), 10, y - 10, 12, box);
    placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::CHISELED->getData()), 10, y - 10, 13, box);
    placeBlock(level, Blocks::sandstone->getBlockState(SandStoneBlock::SMOOTH->getData()), 10, y - 11, 13, box);

    arrayWithLength<const Direction*> faces = Direction::Plane::HORIZONTAL->getFaces();
    for (const Direction** it = faces.data; it != Direction::Plane::HORIZONTAL->getFaces().data + faces.length; ++it)
    {
        const Direction* direction = *it;
        if (!mHasPlacedChest[direction->get2DDataValue()])
        {
            int x = direction->getStepX() * 2;
            int z = direction->getStepZ() * 2;
            mHasPlacedChest[direction->get2DDataValue()] = createChest(level, box, random, x + 10, y - 11, z + 10,
                                                                       &BuiltInLootTables::DESERT_PYRAMID);
        }
    }
    return true;
}

StructurePiece* ScatteredFeaturePieces::DesertPyramidPiece::Create()
{
    return new DesertPyramidPiece();
}
EStructurePiece ScatteredFeaturePieces::DesertPyramidPiece::GetType()
{
    return eStructurePiece_DesertPyramidPiece;
}

ScatteredFeaturePieces::Igloo::Igloo() : ScatteredFeaturePiece()
{
}

ScatteredFeaturePieces::Igloo::Igloo(Level* level, Random& random, int x, int z)
    : ScatteredFeaturePiece(level, random, x, 64, z, 7, 5, 8)
{
    mPlaceSettings.setIgnoreBlock(Blocks::structure_void);
    mHasBasement = random.nextDouble() < 0.5;
    mBasementDepth = random.nextInt(8) + 4;
}

bool ScatteredFeaturePieces::Igloo::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (!updateAverageGroundHeight(level, box, -1, true))
        return false;

    BlockPos origin(getWorldX(0, 0) + 2, getWorldY(0), getWorldZ(0, 0) + 2);
    mPlaceSettings.setBoundingBox(box);
    mPlaceSettings.setRotation(getRotation());

    const StructureTemplate* top = Minecraft::GetInstance()->getStructureManager()->get(
        eMinecraftStructureTemplate_Igloo_Top);
    top->placeInWorld(level, origin, &mPlaceSettings);

    if (!mHasBasement)
    {
        BlockPos snowPos = StructureTemplate::calculateRelativePosition(&mPlaceSettings, BlockPos(3, 0, 5));
        level->setBlock(origin.offset(snowPos), Blocks::snow->defaultBlockState(), Block::UPDATE_ALL, false);
        return true;
    }

    const StructureTemplate* middle = Minecraft::GetInstance()->getStructureManager()->get(
        eMinecraftStructureTemplate_Igloo_Middle);
    const StructureTemplate* bottom = Minecraft::GetInstance()->getStructureManager()->get(
        eMinecraftStructureTemplate_Igloo_Bottom);

    int basementDepth = mBasementDepth;
    for (int i = 0; i < basementDepth; ++i)
    {
        BlockPos connection = top->calculateConnectedPosition(&mPlaceSettings, BlockPos(3, -1 - i * 3, 5),
                                                              &mPlaceSettings, BlockPos(1, 2, 1));
        middle->placeInWorld(level, origin.offset(connection), &mPlaceSettings);
    }

    BlockPos connection = top->calculateConnectedPosition(&mPlaceSettings, BlockPos(3, -1 - basementDepth * 3, 5),
                                                          &mPlaceSettings, BlockPos(3, 5, 7));
    BlockPos bottomPos = origin.offset(connection);
    bottom->placeInWorld(level, bottomPos, &mPlaceSettings);

    StructureTemplate::MarkerMap markers = bottom->getMarkers(bottomPos, &mPlaceSettings);
    for (StructureTemplate::MarkerMap::const_iterator it = markers.begin(); it != markers.end(); ++it)
    {
        if (it->second == L"chest")
        {
            BlockPos chestPos = it->first;
            level->setBlock(chestPos, Blocks::air->defaultBlockState(), Block::UPDATE_ALL, false);
            boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(chestPos.below());
            if (blockEntity != NULL && eTYPE_DERIVED_FROM(BlockEntityType::CHEST, blockEntity->GetType()))
            {
                boost::shared_ptr<ChestBlockEntity> chest = boost::static_pointer_cast<ChestBlockEntity>(blockEntity);
                chest->setLootTable(&BuiltInLootTables::IGLOO_CHEST, static_cast<long long>(random.nextInt()));
            }
        }
    }
    return true;
}

void ScatteredFeaturePieces::Igloo::addAdditionalSaveData(DataOutputStream* output)
{
    ScatteredFeaturePiece::addAdditionalSaveData(output);
    output->writeBoolean(mHasBasement);
    output->writeInt(mBasementDepth);
}

void ScatteredFeaturePieces::Igloo::readAdditionalSaveData(DataInputStream* input, StructureManager* structureManager)
{
    ScatteredFeaturePiece::readAdditionalSaveData(input, structureManager);
    if (StructureFeatureIO::getVersion() >= 3)
    {
        mHasBasement = input->readBoolean();
        mBasementDepth = input->readInt();
    }
}

StructurePiece* ScatteredFeaturePieces::Igloo::Create()
{
    return new Igloo();
}
EStructurePiece ScatteredFeaturePieces::Igloo::GetType()
{
    return eStructurePiece_Igloo;
}

void ScatteredFeaturePieces::JunglePyramidPiece::MossStoneSelector::next(Random& random, int x, int y, int z, bool edge)
{
    if (random.nextFloat() < 0.4f)
        mNext = Blocks::cobblestone->defaultBlockState();
    else
        mNext = Blocks::mossy_cobblestone->defaultBlockState();
}

ScatteredFeaturePieces::JunglePyramidPiece::JunglePyramidPiece() : ScatteredFeaturePiece()
{
}

ScatteredFeaturePieces::JunglePyramidPiece::JunglePyramidPiece(Level* level, Random& random, int x, int z)
    : ScatteredFeaturePiece(level, random, x, 64, z, 12, 10, 15)
{
    mPlacedMainChest = false;
    mPlacedHiddenChest = false;
    mPlacedTrap1 = false;
    mPlacedTrap2 = false;
}

void ScatteredFeaturePieces::JunglePyramidPiece::addAdditionalSaveData(DataOutputStream* output)
{
    ScatteredFeaturePiece::addAdditionalSaveData(output);
    output->writeBoolean(mPlacedMainChest);
    output->writeBoolean(mPlacedHiddenChest);
    output->writeBoolean(mPlacedTrap1);
    output->writeBoolean(mPlacedTrap2);
}

void ScatteredFeaturePieces::JunglePyramidPiece::readAdditionalSaveData(DataInputStream* input,
                                                                        StructureManager* structureManager)
{
    ScatteredFeaturePiece::readAdditionalSaveData(input, structureManager);
    mPlacedMainChest = input->readBoolean();
    mPlacedHiddenChest = input->readBoolean();
    mPlacedTrap1 = input->readBoolean();
    mPlacedTrap2 = input->readBoolean();
}

bool ScatteredFeaturePieces::JunglePyramidPiece::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (!updateAverageGroundHeight(level, box, 0, true))
        return false;

    generateBox(level, box, 0, -4, 0, mWidth - 1, 0, mDepth - 1, false, random, mStoneSelector);
    generateBox(level, box, 2, 1, 2, 9, 2, 2, false, random, mStoneSelector);
    generateBox(level, box, 2, 1, 12, 9, 2, 12, false, random, mStoneSelector);
    generateBox(level, box, 2, 1, 3, 2, 2, 11, false, random, mStoneSelector);
    generateBox(level, box, 9, 1, 3, 9, 2, 11, false, random, mStoneSelector);
    generateBox(level, box, 1, 3, 1, 10, 6, 1, false, random, mStoneSelector);
    generateBox(level, box, 1, 3, 13, 10, 6, 13, false, random, mStoneSelector);
    generateBox(level, box, 1, 3, 2, 1, 6, 12, false, random, mStoneSelector);
    generateBox(level, box, 10, 3, 2, 10, 6, 12, false, random, mStoneSelector);
    generateBox(level, box, 2, 3, 2, 9, 3, 12, false, random, mStoneSelector);
    generateBox(level, box, 2, 6, 2, 9, 6, 12, false, random, mStoneSelector);
    generateBox(level, box, 3, 7, 3, 8, 7, 11, false, random, mStoneSelector);
    generateBox(level, box, 4, 8, 4, 7, 8, 10, false, random, mStoneSelector);
    generateAirBox(level, box, 3, 1, 3, 8, 2, 11);
    generateAirBox(level, box, 4, 3, 6, 7, 3, 9);
    generateAirBox(level, box, 2, 4, 2, 9, 5, 12);
    generateAirBox(level, box, 4, 6, 5, 7, 6, 9);
    generateAirBox(level, box, 5, 7, 6, 6, 7, 8);
    generateAirBox(level, box, 5, 1, 2, 6, 2, 2);
    generateAirBox(level, box, 5, 2, 12, 6, 2, 12);
    generateAirBox(level, box, 5, 5, 1, 6, 5, 1);
    generateAirBox(level, box, 5, 5, 13, 6, 5, 13);
    placeBlock(level, Blocks::air->defaultBlockState(), 1, 5, 5, box);
    placeBlock(level, Blocks::air->defaultBlockState(), 10, 5, 5, box);
    placeBlock(level, Blocks::air->defaultBlockState(), 1, 5, 9, box);
    placeBlock(level, Blocks::air->defaultBlockState(), 10, 5, 9, box);

    for (int z = 0; z <= 14; z += 14)
    {
        generateBox(level, box, 2, 4, z, 2, 5, z, false, random, mStoneSelector);
        generateBox(level, box, 4, 4, z, 4, 5, z, false, random, mStoneSelector);
        generateBox(level, box, 7, 4, z, 7, 5, z, false, random, mStoneSelector);
        generateBox(level, box, 9, 4, z, 9, 5, z, false, random, mStoneSelector);
    }
    generateBox(level, box, 5, 6, 0, 6, 6, 0, false, random, mStoneSelector);

    for (int x = 0; x <= 11; x += 11)
    {
        for (int z = 2; z <= 12; z += 2)
            generateBox(level, box, x, 4, z, x, 5, z, false, random, mStoneSelector);
        generateBox(level, box, x, 6, 5, x, 6, 5, false, random, mStoneSelector);
        generateBox(level, box, x, 6, 9, x, 6, 9, false, random, mStoneSelector);
    }

    generateBox(level, box, 2, 7, 2, 2, 9, 2, false, random, mStoneSelector);
    generateBox(level, box, 9, 7, 2, 9, 9, 2, false, random, mStoneSelector);
    generateBox(level, box, 2, 7, 12, 2, 9, 12, false, random, mStoneSelector);
    generateBox(level, box, 9, 7, 12, 9, 9, 12, false, random, mStoneSelector);
    generateBox(level, box, 4, 9, 4, 4, 9, 4, false, random, mStoneSelector);
    generateBox(level, box, 7, 9, 4, 7, 9, 4, false, random, mStoneSelector);
    generateBox(level, box, 4, 9, 10, 4, 9, 10, false, random, mStoneSelector);
    generateBox(level, box, 7, 9, 10, 7, 9, 10, false, random, mStoneSelector);
    generateBox(level, box, 5, 9, 7, 6, 9, 7, false, random, mStoneSelector);

    const BlockState* stairsEast = Blocks::stone_stairs->defaultBlockState()->setValue(StairBlock::FACING,
                                                                                       Direction::EAST);
    const BlockState* stairsWest = Blocks::stone_stairs->defaultBlockState()->setValue(StairBlock::FACING,
                                                                                       Direction::WEST);
    const BlockState* stairsSouth = Blocks::stone_stairs->defaultBlockState()->setValue(StairBlock::FACING,
                                                                                        Direction::SOUTH);
    const BlockState* stairsNorth = Blocks::stone_stairs->defaultBlockState()->setValue(StairBlock::FACING,
                                                                                        Direction::NORTH);
    placeBlock(level, stairsNorth, 5, 9, 6, box);
    placeBlock(level, stairsNorth, 6, 9, 6, box);
    placeBlock(level, stairsSouth, 5, 9, 8, box);
    placeBlock(level, stairsSouth, 6, 9, 8, box);
    placeBlock(level, stairsNorth, 4, 0, 0, box);
    placeBlock(level, stairsNorth, 5, 0, 0, box);
    placeBlock(level, stairsNorth, 6, 0, 0, box);
    placeBlock(level, stairsNorth, 7, 0, 0, box);
    placeBlock(level, stairsNorth, 4, 1, 8, box);
    placeBlock(level, stairsNorth, 4, 2, 9, box);
    placeBlock(level, stairsNorth, 4, 3, 10, box);
    placeBlock(level, stairsNorth, 7, 1, 8, box);
    placeBlock(level, stairsNorth, 7, 2, 9, box);
    placeBlock(level, stairsNorth, 7, 3, 10, box);
    generateBox(level, box, 4, 1, 9, 4, 1, 9, false, random, mStoneSelector);
    generateBox(level, box, 7, 1, 9, 7, 1, 9, false, random, mStoneSelector);
    generateBox(level, box, 4, 1, 10, 7, 2, 10, false, random, mStoneSelector);
    generateBox(level, box, 5, 4, 5, 6, 4, 5, false, random, mStoneSelector);
    placeBlock(level, stairsEast, 4, 4, 5, box);
    placeBlock(level, stairsWest, 7, 4, 5, box);

    for (int step = 0; step < 4; ++step)
    {
        placeBlock(level, stairsSouth, 5, 0 - step, 6 + step, box);
        placeBlock(level, stairsSouth, 6, 0 - step, 6 + step, box);
        generateAirBox(level, box, 5, 0 - step, 7 + step, 6, 0 - step, 9 + step);
    }

    generateAirBox(level, box, 1, -3, 12, 10, -1, 13);
    generateAirBox(level, box, 1, -3, 1, 3, -1, 13);
    generateAirBox(level, box, 1, -3, 1, 9, -1, 5);
    for (int z = 1; z <= 13; z += 2)
        generateBox(level, box, 1, -3, z, 1, -2, z, false, random, mStoneSelector);
    for (int z = 2; z <= 12; z += 2)
        generateBox(level, box, 1, -1, z, 3, -1, z, false, random, mStoneSelector);
    generateBox(level, box, 2, -2, 1, 5, -2, 1, false, random, mStoneSelector);
    generateBox(level, box, 7, -2, 1, 9, -2, 1, false, random, mStoneSelector);
    generateBox(level, box, 6, -3, 1, 6, -3, 1, false, random, mStoneSelector);
    generateBox(level, box, 6, -1, 1, 6, -1, 1, false, random, mStoneSelector);

    placeBlock(level,
               Blocks::tripwire_hook->defaultBlockState()
                   ->setValue(TripWireHookBlock::FACING, Direction::EAST)
                   ->setValue(TripWireHookBlock::ATTACHED, true),
               1, -3, 8, box);
    placeBlock(level,
               Blocks::tripwire_hook->defaultBlockState()
                   ->setValue(TripWireHookBlock::FACING, Direction::WEST)
                   ->setValue(TripWireHookBlock::ATTACHED, true),
               4, -3, 8, box);
    placeBlock(level, Blocks::tripwire->defaultBlockState()->setValue(TripWireBlock::ATTACHED, true), 2, -3, 8, box);
    placeBlock(level, Blocks::tripwire->defaultBlockState()->setValue(TripWireBlock::ATTACHED, true), 3, -3, 8, box);
    placeBlock(level, Blocks::redstone_wire->defaultBlockState(), 5, -3, 7, box);
    placeBlock(level, Blocks::redstone_wire->defaultBlockState(), 5, -3, 6, box);
    placeBlock(level, Blocks::redstone_wire->defaultBlockState(), 5, -3, 5, box);
    placeBlock(level, Blocks::redstone_wire->defaultBlockState(), 5, -3, 4, box);
    placeBlock(level, Blocks::redstone_wire->defaultBlockState(), 5, -3, 3, box);
    placeBlock(level, Blocks::redstone_wire->defaultBlockState(), 5, -3, 2, box);
    placeBlock(level, Blocks::redstone_wire->defaultBlockState(), 5, -3, 1, box);
    placeBlock(level, Blocks::redstone_wire->defaultBlockState(), 4, -3, 1, box);
    placeBlock(level, Blocks::mossy_cobblestone->defaultBlockState(), 3, -3, 1, box);
    if (!mPlacedTrap1)
    {
        mPlacedTrap1 = createDispenser(level, box, random, 3, -2, 1, Direction::NORTH,
                                       &BuiltInLootTables::JUNGLE_TEMPLE_DISPENSER);
        if (mPlacedTrap1)
            setDirty();
    }

    placeBlock(level, Blocks::vine->defaultBlockState()->setValue(VineBlock::SOUTH, true), 3, -2, 2, box);
    placeBlock(level,
               Blocks::tripwire_hook->defaultBlockState()
                   ->setValue(TripWireHookBlock::FACING, Direction::NORTH)
                   ->setValue(TripWireHookBlock::ATTACHED, true),
               7, -3, 1, box);
    placeBlock(level,
               Blocks::tripwire_hook->defaultBlockState()
                   ->setValue(TripWireHookBlock::FACING, Direction::SOUTH)
                   ->setValue(TripWireHookBlock::ATTACHED, true),
               7, -3, 5, box);
    placeBlock(level, Blocks::tripwire->defaultBlockState()->setValue(TripWireBlock::ATTACHED, true), 7, -3, 2, box);
    placeBlock(level, Blocks::tripwire->defaultBlockState()->setValue(TripWireBlock::ATTACHED, true), 7, -3, 3, box);
    placeBlock(level, Blocks::tripwire->defaultBlockState()->setValue(TripWireBlock::ATTACHED, true), 7, -3, 4, box);
    placeBlock(level, Blocks::redstone_wire->defaultBlockState(), 8, -3, 6, box);
    placeBlock(level, Blocks::redstone_wire->defaultBlockState(), 9, -3, 6, box);
    placeBlock(level, Blocks::redstone_wire->defaultBlockState(), 9, -3, 5, box);
    placeBlock(level, Blocks::mossy_cobblestone->defaultBlockState(), 9, -3, 4, box);
    placeBlock(level, Blocks::redstone_wire->defaultBlockState(), 9, -2, 4, box);
    if (!mPlacedTrap2)
    {
        mPlacedTrap2 = createDispenser(level, box, random, 9, -2, 3, Direction::WEST,
                                       &BuiltInLootTables::JUNGLE_TEMPLE_DISPENSER);
        if (mPlacedTrap2)
            setDirty();
    }

    placeBlock(level, Blocks::vine->defaultBlockState()->setValue(VineBlock::EAST, true), 8, -1, 3, box);
    placeBlock(level, Blocks::vine->defaultBlockState()->setValue(VineBlock::EAST, true), 8, -2, 3, box);
    if (!mPlacedMainChest)
    {
        mPlacedMainChest = createChest(level, box, random, 8, -3, 3, &BuiltInLootTables::JUNGLE_TEMPLE);
        if (mPlacedMainChest)
            setDirty();
    }

    placeBlock(level, Blocks::mossy_cobblestone->defaultBlockState(), 9, -3, 2, box);
    placeBlock(level, Blocks::mossy_cobblestone->defaultBlockState(), 8, -3, 1, box);
    placeBlock(level, Blocks::mossy_cobblestone->defaultBlockState(), 4, -3, 5, box);
    placeBlock(level, Blocks::mossy_cobblestone->defaultBlockState(), 5, -2, 5, box);
    placeBlock(level, Blocks::mossy_cobblestone->defaultBlockState(), 5, -1, 5, box);
    placeBlock(level, Blocks::mossy_cobblestone->defaultBlockState(), 6, -3, 5, box);
    placeBlock(level, Blocks::mossy_cobblestone->defaultBlockState(), 7, -2, 5, box);
    placeBlock(level, Blocks::mossy_cobblestone->defaultBlockState(), 7, -1, 5, box);
    placeBlock(level, Blocks::mossy_cobblestone->defaultBlockState(), 8, -3, 5, box);
    generateBox(level, box, 9, -1, 1, 9, -1, 5, false, random, mStoneSelector);
    generateAirBox(level, box, 8, -3, 8, 10, -1, 10);
    placeBlock(level, Blocks::stonebrick->getBlockState(StoneBrickBlock::CHISELED_META), 8, -2, 11, box);
    placeBlock(level, Blocks::stonebrick->getBlockState(StoneBrickBlock::CHISELED_META), 9, -2, 11, box);
    placeBlock(level, Blocks::stonebrick->getBlockState(StoneBrickBlock::CHISELED_META), 10, -2, 11, box);

    const BlockState* lever = Blocks::lever->defaultBlockState()->setValue(LeverBlock::FACING,
                                                                           LeverBlock::LeverDirection::NORTH);
    placeBlock(level, lever, 8, -2, 12, box);
    placeBlock(level, lever, 9, -2, 12, box);
    placeBlock(level, lever, 10, -2, 12, box);
    generateBox(level, box, 8, -3, 8, 8, -3, 10, false, random, mStoneSelector);
    generateBox(level, box, 10, -3, 8, 10, -3, 10, false, random, mStoneSelector);
    placeBlock(level, Blocks::mossy_cobblestone->defaultBlockState(), 10, -2, 9, box);
    placeBlock(level, Blocks::redstone_wire->defaultBlockState(), 8, -2, 9, box);
    placeBlock(level, Blocks::redstone_wire->defaultBlockState(), 8, -2, 10, box);
    placeBlock(level, Blocks::redstone_wire->defaultBlockState(), 10, -1, 9, box);
    placeBlock(level, Blocks::sticky_piston->defaultBlockState()->setValue(PistonBaseBlock::FACING, Direction::UP), 9,
               -2, 8, box);
    placeBlock(level, Blocks::sticky_piston->defaultBlockState()->setValue(PistonBaseBlock::FACING, Direction::WEST),
               10, -2, 8, box);
    placeBlock(level, Blocks::sticky_piston->defaultBlockState()->setValue(PistonBaseBlock::FACING, Direction::WEST),
               10, -1, 8, box);
    placeBlock(level,
               Blocks::unpowered_repeater->defaultBlockState()->setValue(HorizontalDirectionalBlock::FACING_PROPERTY,
                                                                         Direction::NORTH),
               10, -2, 10, box);
    if (!mPlacedHiddenChest)
    {
        mPlacedHiddenChest = createChest(level, box, random, 9, -3, 10, &BuiltInLootTables::JUNGLE_TEMPLE);
        if (mPlacedHiddenChest)
            setDirty();
    }
    return true;
}

void ScatteredFeaturePieces::JunglePyramidPiece::staticCtor()
{
    mStoneSelector = new MossStoneSelector();
}

StructurePiece* ScatteredFeaturePieces::JunglePyramidPiece::Create()
{
    return new JunglePyramidPiece();
}
EStructurePiece ScatteredFeaturePieces::JunglePyramidPiece::GetType()
{
    return eStructurePiece_JunglePyramidPiece;
}

ScatteredFeaturePieces::SwamplandHut::SwamplandHut() : ScatteredFeaturePiece()
{
    mHasWitch = false;
}

ScatteredFeaturePieces::SwamplandHut::SwamplandHut(Level* level, Random& random, int x, int z)
    : ScatteredFeaturePiece(level, random, x, 64, z, 7, 7, 9)
{
    mHasWitch = false;
}

void ScatteredFeaturePieces::SwamplandHut::addAdditionalSaveData(DataOutputStream* output)
{
    ScatteredFeaturePiece::addAdditionalSaveData(output);
    output->writeBoolean(mHasWitch);
}

void ScatteredFeaturePieces::SwamplandHut::readAdditionalSaveData(DataInputStream* input,
                                                                  StructureManager* structureManager)
{
    ScatteredFeaturePiece::readAdditionalSaveData(input, structureManager);
    mHasWitch = input->readBoolean();
}

bool ScatteredFeaturePieces::SwamplandHut::postProcess(Level* level, Random& random, BoundingBox* box)
{
    if (!updateAverageGroundHeight(level, box, 0, true))
        return false;

    generateBox(level, box, 1, 1, 1, 5, 1, 7, Blocks::planks->getBlockState(PlanksBlock::Variant::SPRUCE->getData()),
                Blocks::planks->getBlockState(PlanksBlock::Variant::SPRUCE->getData()), false);
    generateBox(level, box, 1, 4, 2, 5, 4, 7, Blocks::planks->getBlockState(PlanksBlock::Variant::SPRUCE->getData()),
                Blocks::planks->getBlockState(PlanksBlock::Variant::SPRUCE->getData()), false);
    generateBox(level, box, 2, 1, 0, 4, 1, 0, Blocks::planks->getBlockState(PlanksBlock::Variant::SPRUCE->getData()),
                Blocks::planks->getBlockState(PlanksBlock::Variant::SPRUCE->getData()), false);
    generateBox(level, box, 2, 2, 2, 3, 3, 2, Blocks::planks->getBlockState(PlanksBlock::Variant::SPRUCE->getData()),
                Blocks::planks->getBlockState(PlanksBlock::Variant::SPRUCE->getData()), false);
    generateBox(level, box, 1, 2, 3, 1, 3, 6, Blocks::planks->getBlockState(PlanksBlock::Variant::SPRUCE->getData()),
                Blocks::planks->getBlockState(PlanksBlock::Variant::SPRUCE->getData()), false);
    generateBox(level, box, 5, 2, 3, 5, 3, 6, Blocks::planks->getBlockState(PlanksBlock::Variant::SPRUCE->getData()),
                Blocks::planks->getBlockState(PlanksBlock::Variant::SPRUCE->getData()), false);
    generateBox(level, box, 2, 2, 7, 4, 3, 7, Blocks::planks->getBlockState(PlanksBlock::Variant::SPRUCE->getData()),
                Blocks::planks->getBlockState(PlanksBlock::Variant::SPRUCE->getData()), false);
    generateBox(level, box, 1, 0, 2, 1, 3, 2, Blocks::log->defaultBlockState(), Blocks::log->defaultBlockState(),
                false);
    generateBox(level, box, 5, 0, 2, 5, 3, 2, Blocks::log->defaultBlockState(), Blocks::log->defaultBlockState(),
                false);
    generateBox(level, box, 1, 0, 7, 1, 3, 7, Blocks::log->defaultBlockState(), Blocks::log->defaultBlockState(),
                false);
    generateBox(level, box, 5, 0, 7, 5, 3, 7, Blocks::log->defaultBlockState(), Blocks::log->defaultBlockState(),
                false);
    placeBlock(level, Blocks::fence->defaultBlockState(), 2, 3, 2, box);
    placeBlock(level, Blocks::fence->defaultBlockState(), 3, 3, 7, box);
    placeBlock(level, Blocks::air->defaultBlockState(), 1, 3, 4, box);
    placeBlock(level, Blocks::air->defaultBlockState(), 5, 3, 4, box);
    placeBlock(level, Blocks::air->defaultBlockState(), 5, 3, 5, box);

    placeBlock(level,
               Blocks::flower_pot->defaultBlockState()->setValue<const FlowerPotBlock::Contents*>(
                   FlowerPotBlock::CONTENTS, FlowerPotBlock::Contents::MUSHROOM_RED),
               1, 3, 5, box);
    BlockPos flowerPotPos(getWorldX(1, 5), getWorldY(3), getWorldZ(1, 5));
    {
        boost::shared_ptr<FlowerPotBlockEntity> flowerPot = boost::dynamic_pointer_cast<FlowerPotBlockEntity>(
            level->getBlockEntity(flowerPotPos));
        if (flowerPot != NULL)
            flowerPot->setItemInstance(not_null_ptr<ItemInstance>(new ItemInstance(Blocks::red_mushroom)));
    }

    placeBlock(level, Blocks::crafting_table->defaultBlockState(), 3, 2, 6, box);
    placeBlock(level, Blocks::cauldron->defaultBlockState(), 4, 2, 6, box);
    placeBlock(level, Blocks::fence->defaultBlockState(), 1, 2, 1, box);
    placeBlock(level, Blocks::fence->defaultBlockState(), 5, 2, 1, box);

    const BlockState* stairsNorth = Blocks::spruce_stairs->defaultBlockState()->setValue(StairBlock::FACING,
                                                                                         Direction::NORTH);
    const BlockState* stairsEast = Blocks::spruce_stairs->defaultBlockState()->setValue(StairBlock::FACING,
                                                                                        Direction::EAST);
    const BlockState* stairsWest = Blocks::spruce_stairs->defaultBlockState()->setValue(StairBlock::FACING,
                                                                                        Direction::WEST);
    const BlockState* stairsSouth = Blocks::spruce_stairs->defaultBlockState()->setValue(StairBlock::FACING,
                                                                                         Direction::SOUTH);
    generateBox(level, box, 0, 4, 1, 6, 4, 1, stairsNorth, stairsNorth, false);
    generateBox(level, box, 0, 4, 2, 0, 4, 7, stairsEast, stairsEast, false);
    generateBox(level, box, 6, 4, 2, 6, 4, 7, stairsWest, stairsWest, false);
    generateBox(level, box, 0, 4, 8, 6, 4, 8, stairsSouth, stairsSouth, false);
    fillColumnDown(level, Blocks::log->defaultBlockState(), 1, -1, 2, box);
    fillColumnDown(level, Blocks::log->defaultBlockState(), 5, -1, 2, box);
    fillColumnDown(level, Blocks::log->defaultBlockState(), 1, -1, 7, box);
    fillColumnDown(level, Blocks::log->defaultBlockState(), 5, -1, 7, box);

    if (!mHasWitch)
    {
        int x = getWorldX(2, 5);
        int y = getWorldY(2);
        int z = getWorldZ(2, 5);
        if (box->isInside(BlockPos(x, y, z)))
        {
            setDirty();
            mHasWitch = true;
            boost::shared_ptr<Witch> witch(new Witch(level));
            witch->setPersistenceRequired(true);
            witch->moveTo((double)x + 0.5, (double)y, (double)z + 0.5, 0.0f, 0.0f);
            witch->finalizeMobSpawn(level->getCurrentDifficultyAt(BlockPos(x, y, z)), NULL, L"");
            level->addEntity(witch);
        }
    }
    return true;
}

StructurePiece* ScatteredFeaturePieces::SwamplandHut::Create()
{
    return new SwamplandHut();
}
EStructurePiece ScatteredFeaturePieces::SwamplandHut::GetType()
{
    return eStructurePiece_SwamplandHut;
}

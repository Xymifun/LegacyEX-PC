#include "game/world/generation/FossilFeature.hpp"

#include "game/core/BoundingBox.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/StructureManager.hpp"
#include "game/world/generation/StructurePlaceSettings.hpp"
#include "game/world/generation/StructureTemplate.hpp"

static const eMinecraftStructureTemplate FOSSILS[8] = {
    eMinecraftStructureTemplate_Fossil_Spine01, eMinecraftStructureTemplate_Fossil_Spine02,
    eMinecraftStructureTemplate_Fossil_Spine03, eMinecraftStructureTemplate_Fossil_Spine04,
    eMinecraftStructureTemplate_Fossil_Skull01, eMinecraftStructureTemplate_Fossil_Skull02,
    eMinecraftStructureTemplate_Fossil_Skull03, eMinecraftStructureTemplate_Fossil_Skull04};

static const eMinecraftStructureTemplate FOSSILS_COAL[8] = {
    eMinecraftStructureTemplate_Fossil_Spine01Coal, eMinecraftStructureTemplate_Fossil_Spine02Coal,
    eMinecraftStructureTemplate_Fossil_Spine03Coal, eMinecraftStructureTemplate_Fossil_Spine04Coal,
    eMinecraftStructureTemplate_Fossil_Skull01Coal, eMinecraftStructureTemplate_Fossil_Skull02Coal,
    eMinecraftStructureTemplate_Fossil_Skull03Coal, eMinecraftStructureTemplate_Fossil_Skull04Coal};

bool FossilFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    Random* placementRandom = level->getChunkAt(origin)->getRandom(0x3ad8025fLL);
    Rotation* rotation = Rotation::VALUES[placementRandom->nextInt(4)];
    const int fossilIndex = placementRandom->nextInt(8);

    Minecraft* minecraft = Minecraft::GetInstance();
    StructureManager* structureManager = minecraft->getStructureManager();
    const StructureTemplate* fossil = structureManager->get(FOSSILS[fossilIndex]);
    const StructureTemplate* fossilCoal = structureManager->get(FOSSILS_COAL[fossilIndex]);

    ChunkPos chunkPos(origin);
    BoundingBox* boundingBox = new BoundingBox(chunkPos.getMinBlockX(), 0, chunkPos.getMinBlockZ(),
                                               chunkPos.getMaxBlockX(), 256, chunkPos.getMaxBlockZ());
    StructurePlaceSettings* settings =
        (new StructurePlaceSettings())->setRotation(rotation)->setBoundingBox(boundingBox)->setRandom(placementRandom);

    const BlockPos size = fossil->getSize(rotation);
    const int xOffset = placementRandom->nextInt(16 - size.x);
    const int zOffset = placementRandom->nextInt(16 - size.z);
    int height = 256;
    for (int x = 0; x < size.x; ++x)
    {
        for (int z = 0; z < size.x; ++z)
        {
            const int columnHeight = level->getHeightmapHeight(origin.x + x + xOffset, origin.z + z + zOffset);
            if (columnHeight < height)
            {
                height = columnHeight;
            }
        }
    }

    int yOffset = height - 15 - placementRandom->nextInt(10);
    if (yOffset < 10)
    {
        yOffset = 10;
    }

    const BlockPos position = origin.offset(xOffset, yOffset, zOffset);
    const BlockPos transformed = fossil->getZeroPositionWithTransform(position, Mirror::NONE, rotation);

    settings->setIntegrity(0.9f);
    fossil->placeInWorld(level, transformed, settings, 20);
    settings->setIntegrity(0.1f);
    fossilCoal->placeInWorld(level, transformed, settings, 20);

    delete settings;

    return true;
}

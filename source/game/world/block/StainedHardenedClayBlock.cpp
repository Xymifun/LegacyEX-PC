#include "game/world/block/StainedHardenedClayBlock.hpp"

#include "game/core/DyeColor.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"

static const MaterialColor* gStainedHardenedClayColors[16];

void StainedHardenedClayBlock::staticCtor()
{
    gStainedHardenedClayColors[0] = MaterialColor::TERRACOTTA_WHITE;
    gStainedHardenedClayColors[1] = MaterialColor::TERRACOTTA_ORANGE;
    gStainedHardenedClayColors[2] = MaterialColor::TERRACOTTA_MAGENTA;
    gStainedHardenedClayColors[3] = MaterialColor::TERRACOTTA_LIGHT_BLUE;
    gStainedHardenedClayColors[4] = MaterialColor::TERRACOTTA_YELLOW;
    gStainedHardenedClayColors[5] = MaterialColor::TERRACOTTA_LIGHT_GREEN;
    gStainedHardenedClayColors[6] = MaterialColor::TERRACOTTA_PINK;
    gStainedHardenedClayColors[7] = MaterialColor::TERRACOTTA_GREY;
    gStainedHardenedClayColors[8] = MaterialColor::TERRACOTTA_SILVER;
    gStainedHardenedClayColors[9] = MaterialColor::TERRACOTTA_CYAN;
    gStainedHardenedClayColors[10] = MaterialColor::TERRACOTTA_PURPLE;
    gStainedHardenedClayColors[11] = MaterialColor::TERRACOTTA_BLUE;
    gStainedHardenedClayColors[12] = MaterialColor::TERRACOTTA_BROWN;
    gStainedHardenedClayColors[13] = MaterialColor::TERRACOTTA_GREEN;
    gStainedHardenedClayColors[14] = MaterialColor::TERRACOTTA_RED;
    gStainedHardenedClayColors[15] = MaterialColor::TERRACOTTA_BLACK;
}

StainedHardenedClayBlock::StainedHardenedClayBlock() : ColoredBlock(Material::STONE)
{
}

const MaterialColor* StainedHardenedClayBlock::getMapColor(const BlockState* state, LevelSource* levelSource,
                                                           const BlockPos& pos)
{
    return gStainedHardenedClayColors[state->getValue(ColoredBlock::COLOR)->getBlockData()];
}

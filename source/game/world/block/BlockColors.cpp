#include "game/world/block/BlockColors.hpp"

#include <cstddef>

#include <boost/shared_ptr.hpp>

#include "game/core/ColourTable.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/FoliageColor.hpp"
#include "game/core/GrassColor.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/core/Minecraft.hpp"
#include "game/world/biome/BiomeColors.hpp"
#include "game/world/block/DoublePlantBlock.hpp"
#include "game/world/block/FlowerPotBlockEntity.hpp"
#include "game/world/block/OldLeafBlock.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/RedStoneWireBlock.hpp"
#include "game/world/block/ShulkerBoxBlock.hpp"
#include "game/world/block/StemBlock.hpp"
#include "game/world/block/TallGrassBlock.hpp"

BlockColors::BlockColors()
{
    mBlockColorIds = new IdMapper<BlockColor*>();
}

void BlockColors::registerColor(BlockColor* color, arrayWithLength<Block*> blocks)
{
    for (unsigned int i = 0; i < blocks.length; i++)
    {
        int id = Block::getId(blocks.data[i]);
        mBlockColorIds->addMapping(color, id);
    }
}

BlockColors* BlockColors::createDefault()
{
    BlockColors* blockColors = new BlockColors();

    class DoublePlant_BlockColor : public BlockColor
    {
    public:
        virtual int getColor(const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex)
        {
            DoublePlantBlock::Variant* variant = state->getValue(DoublePlantBlock::VARIANT);
            if (levelSource != NULL && pos != NULL && (variant == gDoublePlantGrass || variant == gDoublePlantFern))
            {
                DoublePlantBlock::Half* half = state->getValue(DoublePlantBlock::HALF);
                return BiomeColors::getAverageGrassColor(levelSource,
                                                         half == DoublePlantBlock::UPPER_HALF ? pos->below() : *pos);
            }

            return -1;
        }
    };

    Block* doublePlantBlocks[] = {Blocks::double_plant};
    blockColors->registerColor(new DoublePlant_BlockColor(), arrayWithLength<Block*>(doublePlantBlocks, 1));

    class FlowerPot_BlockColor : public BlockColor
    {
    public:
        FlowerPot_BlockColor(BlockColors* blockColors) { mBlockColors = blockColors; }

        virtual int getColor(const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex)
        {
            if (levelSource != NULL && pos != NULL)
            {
                boost::shared_ptr<BlockEntity> blockEntity = levelSource->getBlockEntity(*pos);
                boost::shared_ptr<FlowerPotBlockEntity> flowerPot = boost::dynamic_pointer_cast<FlowerPotBlockEntity>(
                    blockEntity);

                if (flowerPot)
                {
                    return mBlockColors->getColor(Block::byItem(flowerPot->getPlantItem())->defaultBlockState(),
                                                  levelSource, pos, tintIndex);
                }
            }

            return -1;
        }

    private:
        BlockColors* mBlockColors;
    };

    Block* flowerPotBlocks[] = {Blocks::flower_pot};
    blockColors->registerColor(new FlowerPot_BlockColor(blockColors), arrayWithLength<Block*>(flowerPotBlocks, 1));

    class Grass_BlockColor : public BlockColor
    {
    public:
        virtual int getColor(const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex)
        {
            if (levelSource == NULL || pos == NULL)
            {
                return GrassColor::get(0.5, 1.0);
            }

            return BiomeColors::getAverageGrassColor(levelSource, *pos);
        }
    };

    Block* grassBlocks[] = {Blocks::grass};
    blockColors->registerColor(new Grass_BlockColor(), arrayWithLength<Block*>(grassBlocks, 1));

    class Leaves1_BlockColor : public BlockColor
    {
    public:
        virtual int getColor(const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex)
        {
            PlanksBlock::Variant* variant = state->getValue(OldLeafBlock::VARIANT);

            if (variant == PlanksBlock::Variant::SPRUCE)
            {
                return FoliageColor::getEvergreenColor();
            }

            if (variant == PlanksBlock::Variant::BIRCH)
            {
                return FoliageColor::getBirchColor();
            }

            if (levelSource == NULL || pos == NULL)
            {
                return FoliageColor::getDefaultColor();
            }

            return BiomeColors::getAverageFoliageColor(levelSource, *pos);
        }
    };

    Block* leavesBlocks[] = {Blocks::leaves};
    blockColors->registerColor(new Leaves1_BlockColor(), arrayWithLength<Block*>(leavesBlocks, 1));

    class Leaves2_BlockColor : public BlockColor
    {
    public:
        virtual int getColor(const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex)
        {
            if (levelSource == NULL || pos == NULL)
            {
                return FoliageColor::getDefaultColor();
            }

            return BiomeColors::getAverageFoliageColor(levelSource, *pos);
        }
    };

    Block* leaves2Blocks[] = {Blocks::leaves2};
    blockColors->registerColor(new Leaves2_BlockColor(), arrayWithLength<Block*>(leaves2Blocks, 1));

    class Water_BlockColor : public BlockColor
    {
    public:
        virtual int getColor(const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex)
        {
            if (levelSource == NULL || pos == NULL)
            {
                return -1;
            }

            return BiomeColors::getAverageWaterColor(levelSource, *pos);
        }
    };

    Block* waterBlocks[] = {Blocks::water, Blocks::flowing_water};
    blockColors->registerColor(new Water_BlockColor(), arrayWithLength<Block*>(waterBlocks, 2));

    class RedstoneWire_BlockColor : public BlockColor
    {
    public:
        virtual int getColor(const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex)
        {
            return RedStoneWireBlock::getColorForData(state->getValue(RedStoneWireBlock::POWER));
        }
    };

    Block* redstoneWireBlocks[] = {Blocks::redstone_wire};
    blockColors->registerColor(new RedstoneWire_BlockColor(), arrayWithLength<Block*>(redstoneWireBlocks, 1));

    class Reeds_BlockColor : public BlockColor
    {
    public:
        virtual int getColor(const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex)
        {
            return 0xFFFFFF;
        }
    };

    Block* reedsBlocks[] = {Blocks::reeds};
    blockColors->registerColor(new Reeds_BlockColor(), arrayWithLength<Block*>(reedsBlocks, 1));

    class Stem_BlockColor : public BlockColor
    {
    public:
        virtual int getColor(const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex)
        {
            int age = state->getValue(StemBlock::AGE);
            return (age * 32) << 16 | (255 - age * 8) << 8 | age * 4;
        }
    };

    Block* stemBlocks[] = {Blocks::melon_stem, Blocks::pumpkin_stem};
    blockColors->registerColor(new Stem_BlockColor(), arrayWithLength<Block*>(stemBlocks, 2));

    class TallGrass_BlockColor : public BlockColor
    {
    public:
        virtual int getColor(const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex)
        {
            if (levelSource == NULL || pos == NULL)
            {
                if (state->getValue(TallGrassBlock::VARIANT) != TallGrassBlock::Variant::DEAD_BUSH)
                {
                    return GrassColor::get(0.5, 1.0);
                }
            }
            else
            {
                return BiomeColors::getAverageGrassColor(levelSource, *pos);
            }

            return 0xFFFFFF;
        }
    };

    Block* tallGrassBlocks[] = {Blocks::tallgrass};
    blockColors->registerColor(new TallGrass_BlockColor(), arrayWithLength<Block*>(tallGrassBlocks, 1));

    class Vine_BlockColor : public BlockColor
    {
    public:
        virtual int getColor(const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex)
        {
            if (levelSource == NULL || pos == NULL)
            {
                return FoliageColor::getDefaultColor();
            }

            return BiomeColors::getAverageFoliageColor(levelSource, *pos);
        }
    };

    Block* vineBlocks[] = {Blocks::vine};
    blockColors->registerColor(new Vine_BlockColor(), arrayWithLength<Block*>(vineBlocks, 1));

    class WaterLily_BlockColor : public BlockColor
    {
    public:
        virtual int getColor(const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex)
        {
            return Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Tile_WaterLily);
        }
    };

    Block* waterLilyBlocks[] = {Blocks::waterlily};
    blockColors->registerColor(new WaterLily_BlockColor(), arrayWithLength<Block*>(waterLilyBlocks, 1));

    class ShulkerBox_BlockColor : public BlockColor
    {
    public:
        virtual int getColor(const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex)
        {
            const DyeColor* dyeColor = static_cast<ShulkerBoxBlock*>(state->getBlock())->getDyeColor();
            ColourTable* colourTable = Minecraft::GetInstance()->getColourTable();
            eMinecraftColour colour = dyeColor->getShulkerColor();
            return colourTable->getColour(colour);
        }
    };

    Block* shulkerBoxBlocks[] = {
        Blocks::white_shulker_box,      Blocks::orange_shulker_box, Blocks::magenta_shulker_box,
        Blocks::light_blue_shulker_box, Blocks::yellow_shulker_box, Blocks::lime_shulker_box,
        Blocks::pink_shulker_box,       Blocks::gray_shulker_box,   Blocks::silver_shulker_box,
        Blocks::cyan_shulker_box,       Blocks::purple_shulker_box, Blocks::blue_shulker_box,
        Blocks::brown_shulker_box,      Blocks::green_shulker_box,  Blocks::red_shulker_box,
        Blocks::black_shulker_box};
    blockColors->registerColor(new ShulkerBox_BlockColor(), arrayWithLength<Block*>(shulkerBoxBlocks, 16));

    return blockColors;
}

int BlockColors::getColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    BlockColor* color = mBlockColorIds->byId(Block::getId(state->getBlock()));

    if (color != NULL)
    {
        return color->getColor(state, NULL, NULL, 0);
    }

    const MaterialColor* mapColor = const_cast<BlockState*>(state)->getMapColor(levelSource, pos);

    if (mapColor != NULL)
    {
        ColourTable* colourTable = Minecraft::GetInstance()->getColourTable();
        return colourTable->getColour(mapColor->col);
    }

    return -1;
}

int BlockColors::getColor(const BlockState* state, LevelSource* levelSource, const BlockPos* pos, int tintIndex)
{
    BlockColor* color = mBlockColorIds->byId(Block::getId(state->getBlock()));

    if (color == NULL)
    {
        return -1;
    }

    return color->getColor(state, levelSource, pos, tintIndex);
}

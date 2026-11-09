#include "game/item/FurnaceRecipes.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcModContent.hpp"
#endif
#include "game/world/block/StoneBrickBlock.hpp"
#include "game/core/DyeColor.hpp"

#include "game/item/FishFoodItem.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/world/block/Blocks.hpp"

FurnaceRecipes* gFurnaceRecipes = 0;

static const int CHARCOAL_DATA = 1;
static const int WET_SPONGE_DATA = 1;
static const int DRY_SPONGE_DATA = 0;

FurnaceRecipes::FurnaceRecipes()
{
    addFurnaceRecipe(Blocks::iron_ore, new ItemInstance(Items::ironIngot), 0.7f);
    addFurnaceRecipe(Blocks::gold_ore, new ItemInstance(Items::goldIngot), 1.0f);
    addFurnaceRecipe(Blocks::diamond_ore, new ItemInstance(Items::diamond), 1.0f);
    addFurnaceRecipe(Blocks::sand, new ItemInstance(Blocks::glass), 0.1f);
    addFurnaceRecipe(Items::porkchop, new ItemInstance(Items::cookedPorkchop), 0.35f);
    addFurnaceRecipe(Items::beef, new ItemInstance(Items::cookedBeef), 0.35f);
    addFurnaceRecipe(Items::chicken, new ItemInstance(Items::cookedChicken), 0.35f);
    addFurnaceRecipe(Items::rabbit, new ItemInstance(Items::cookedRabbit), 0.35f);
    addFurnaceRecipe(Items::mutton, new ItemInstance(Items::cookedMutton), 0.35f);
    addFurnaceRecipe(Items::kelp, new ItemInstance(Items::driedKelp), 0.05f);
    addFurnaceRecipe(Blocks::cobblestone, new ItemInstance(Blocks::stone), 0.1f);
    addFurnaceRecipe(new ItemInstance(Blocks::stonebrick, 1, StoneBrickBlock::DEFAULT_META),
                     new ItemInstance(Blocks::stonebrick, 1, StoneBrickBlock::CRACKED_META), 0.1f);
    addFurnaceRecipe(Items::clayBall, new ItemInstance(Items::brick), 0.3f);
    addFurnaceRecipe(Blocks::clay, new ItemInstance(Blocks::hardened_clay), 0.35f);
    addFurnaceRecipe(Blocks::cactus, new ItemInstance(Items::dye, 1, DyeColor::GREEN->getItemData()), 0.2f);
    addFurnaceRecipe(Blocks::sea_pickle, new ItemInstance(Items::dye, 1, DyeColor::LIME->getItemData()), 0.2f);
    addFurnaceRecipe(Blocks::log, new ItemInstance(Items::coal, 1, CHARCOAL_DATA), 0.15f);
    addFurnaceRecipe(Blocks::log2, new ItemInstance(Items::coal, 1, CHARCOAL_DATA), 0.15f);
    addFurnaceRecipe(Blocks::stripped_log_spruce, new ItemInstance(Items::coal, 1, CHARCOAL_DATA), 0.15f);
    addFurnaceRecipe(Blocks::stripped_log_birch, new ItemInstance(Items::coal, 1, CHARCOAL_DATA), 0.15f);
    addFurnaceRecipe(Blocks::stripped_log_jungle, new ItemInstance(Items::coal, 1, CHARCOAL_DATA), 0.15f);
    addFurnaceRecipe(Blocks::stripped_log_acacia, new ItemInstance(Items::coal, 1, CHARCOAL_DATA), 0.15f);
    addFurnaceRecipe(Blocks::stripped_log_dark_oak, new ItemInstance(Items::coal, 1, CHARCOAL_DATA), 0.15f);
    addFurnaceRecipe(Blocks::stripped_log_oak, new ItemInstance(Items::coal, 1, CHARCOAL_DATA), 0.15f);
    addFurnaceRecipe(Blocks::emerald_ore, new ItemInstance(Items::emerald), 1.0f);
    addFurnaceRecipe(Items::potato, new ItemInstance(Items::bakedPotato), 0.35f);
    addFurnaceRecipe(Blocks::netherrack, new ItemInstance(Items::netherbrick), 0.1f);
    addFurnaceRecipe(new ItemInstance(Blocks::sponge, 1, WET_SPONGE_DATA),
                     new ItemInstance(Blocks::sponge, 1, DRY_SPONGE_DATA), 0.15f);
    addFurnaceRecipe(Items::chorusFruit, new ItemInstance(Items::chorusFruitPopped), 0.1f);

    for (int i = 0; i < 4; ++i)
    {
        FishFoodItem::Type* type = gFishFoodTypes[i];
        if (type->canBeCooked())
            addFurnaceRecipe(new ItemInstance(Items::fish, 1, type->getId()),
                             new ItemInstance(Items::cookedFish, 1, type->getId()), 0.35f);
    }

    addFurnaceRecipe(Blocks::coal_ore, new ItemInstance(Items::coal), 0.1f);
    addFurnaceRecipe(Blocks::redstone_ore, new ItemInstance(Items::redstone), 0.7f);
    addFurnaceRecipe(Blocks::lapis_ore, new ItemInstance(Items::dye, 1, DyeColor::BLUE->getItemData()), 0.2f);
    addFurnaceRecipe(Blocks::quartz_ore, new ItemInstance(Items::quartz), 0.2f);
    addFurnaceRecipe(Items::chainmailHelmet, new ItemInstance(Items::ironNugget), 0.1f);
    addFurnaceRecipe(Items::chainmailChestplate, new ItemInstance(Items::ironNugget), 0.1f);
    addFurnaceRecipe(Items::chainmailLeggings, new ItemInstance(Items::ironNugget), 0.1f);
    addFurnaceRecipe(Items::chainmailBoots, new ItemInstance(Items::ironNugget), 0.1f);
    addFurnaceRecipe(Items::ironPickaxe, new ItemInstance(Items::ironNugget), 0.1f);
    addFurnaceRecipe(Items::ironShovel, new ItemInstance(Items::ironNugget), 0.1f);
    addFurnaceRecipe(Items::ironAxe, new ItemInstance(Items::ironNugget), 0.1f);
    addFurnaceRecipe(Items::ironHoe, new ItemInstance(Items::ironNugget), 0.1f);
    addFurnaceRecipe(Items::ironSword, new ItemInstance(Items::ironNugget), 0.1f);
    addFurnaceRecipe(Items::ironHelmet, new ItemInstance(Items::ironNugget), 0.1f);
    addFurnaceRecipe(Items::ironChestplate, new ItemInstance(Items::ironNugget), 0.1f);
    addFurnaceRecipe(Items::ironLeggings, new ItemInstance(Items::ironNugget), 0.1f);
    addFurnaceRecipe(Items::ironBoots, new ItemInstance(Items::ironNugget), 0.1f);
    addFurnaceRecipe(Items::ironHorseArmor, new ItemInstance(Items::ironNugget), 0.1f);
    addFurnaceRecipe(Items::goldenPickaxe, new ItemInstance(Items::goldNugget), 0.1f);
    addFurnaceRecipe(Items::goldenShovel, new ItemInstance(Items::goldNugget), 0.1f);
    addFurnaceRecipe(Items::goldenAxe, new ItemInstance(Items::goldNugget), 0.1f);
    addFurnaceRecipe(Items::goldenHoe, new ItemInstance(Items::goldNugget), 0.1f);
    addFurnaceRecipe(Items::goldenSword, new ItemInstance(Items::goldNugget), 0.1f);
    addFurnaceRecipe(Items::goldenHelmet, new ItemInstance(Items::goldNugget), 0.1f);
    addFurnaceRecipe(Items::goldenChestplate, new ItemInstance(Items::goldNugget), 0.1f);
    addFurnaceRecipe(Items::goldenLeggings, new ItemInstance(Items::goldNugget), 0.1f);
    addFurnaceRecipe(Items::goldenBoots, new ItemInstance(Items::goldNugget), 0.1f);
    addFurnaceRecipe(Items::goldenHorseArmor, new ItemInstance(Items::goldNugget), 0.1f);
    addFurnaceRecipe(new ItemInstance(Blocks::stained_hardened_clay, 1, DyeColor::WHITE->getBlockData()),
                     new ItemInstance(Blocks::white_glazed_terracotta), 0.1f);
    addFurnaceRecipe(new ItemInstance(Blocks::stained_hardened_clay, 1, DyeColor::ORANGE->getBlockData()),
                     new ItemInstance(Blocks::orange_glazed_terracotta), 0.1f);
    addFurnaceRecipe(new ItemInstance(Blocks::stained_hardened_clay, 1, DyeColor::MAGENTA->getBlockData()),
                     new ItemInstance(Blocks::magenta_glazed_terracotta), 0.1f);
    addFurnaceRecipe(new ItemInstance(Blocks::stained_hardened_clay, 1, DyeColor::LIGHT_BLUE->getBlockData()),
                     new ItemInstance(Blocks::light_blue_glazed_terracotta), 0.1f);
    addFurnaceRecipe(new ItemInstance(Blocks::stained_hardened_clay, 1, DyeColor::YELLOW->getBlockData()),
                     new ItemInstance(Blocks::yellow_glazed_terracotta), 0.1f);
    addFurnaceRecipe(new ItemInstance(Blocks::stained_hardened_clay, 1, DyeColor::LIME->getBlockData()),
                     new ItemInstance(Blocks::lime_glazed_terracotta), 0.1f);
    addFurnaceRecipe(new ItemInstance(Blocks::stained_hardened_clay, 1, DyeColor::PINK->getBlockData()),
                     new ItemInstance(Blocks::pink_glazed_terracotta), 0.1f);
    addFurnaceRecipe(new ItemInstance(Blocks::stained_hardened_clay, 1, DyeColor::GRAY->getBlockData()),
                     new ItemInstance(Blocks::gray_glazed_terracotta), 0.1f);
    addFurnaceRecipe(new ItemInstance(Blocks::stained_hardened_clay, 1, DyeColor::SILVER->getBlockData()),
                     new ItemInstance(Blocks::silver_glazed_terracotta), 0.1f);
    addFurnaceRecipe(new ItemInstance(Blocks::stained_hardened_clay, 1, DyeColor::CYAN->getBlockData()),
                     new ItemInstance(Blocks::cyan_glazed_terracotta), 0.1f);
    addFurnaceRecipe(new ItemInstance(Blocks::stained_hardened_clay, 1, DyeColor::PURPLE->getBlockData()),
                     new ItemInstance(Blocks::purple_glazed_terracotta), 0.1f);
    addFurnaceRecipe(new ItemInstance(Blocks::stained_hardened_clay, 1, DyeColor::BLUE->getBlockData()),
                     new ItemInstance(Blocks::blue_glazed_terracotta), 0.1f);
    addFurnaceRecipe(new ItemInstance(Blocks::stained_hardened_clay, 1, DyeColor::BROWN->getBlockData()),
                     new ItemInstance(Blocks::brown_glazed_terracotta), 0.1f);
    addFurnaceRecipe(new ItemInstance(Blocks::stained_hardened_clay, 1, DyeColor::GREEN->getBlockData()),
                     new ItemInstance(Blocks::green_glazed_terracotta), 0.1f);
    addFurnaceRecipe(new ItemInstance(Blocks::stained_hardened_clay, 1, DyeColor::RED->getBlockData()),
                     new ItemInstance(Blocks::red_glazed_terracotta), 0.1f);
    addFurnaceRecipe(new ItemInstance(Blocks::stained_hardened_clay, 1, DyeColor::BLACK->getBlockData()),
                     new ItemInstance(Blocks::black_glazed_terracotta), 0.1f);
#ifdef MINECRAFT_PC
    PcModContentAddSmelting(this);
#endif
}

bool FurnaceRecipes::isSameItem(not_null_ptr<ItemInstance> item, ItemInstance* recipeItem)
{
    if (item->getItem() != recipeItem->getItem())
    {
        return false;
    }
    if (recipeItem->getAuxValue() != -1 && item->getAuxValue() != recipeItem->getAuxValue())
    {
        return false;
    }
    return true;
}

ItemInstance* FurnaceRecipes::getResult(not_null_ptr<ItemInstance> item)
{
    iterator it;
    for (it = begin(); it != end(); ++it)
    {
        if (isSameItem(item, it->first))
        {
            return it->second;
        }
    }
    return ItemInstance::EMPTY.get();
}

FurnaceRecipes* FurnaceRecipes::getInstance()
{
    return gFurnaceRecipes;
}

float FurnaceRecipes::getRecipeValue(not_null_ptr<ItemInstance> item)
{
    boost::unordered_map<ItemInstance*, float>::iterator it;
    for (it = mRecipeExperience.begin(); it != mRecipeExperience.end(); ++it)
    {
        if (isSameItem(item, it->first))
        {
            return it->second;
        }
    }
    return 0.0f;
}

void FurnaceRecipes::addFurnaceRecipe(ItemInstance* item, ItemInstance* result, float experience)
{
    (*this)[item] = result;
    mRecipeExperience[result] = experience;
}

void FurnaceRecipes::addFurnaceRecipe(Item* item, ItemInstance* result, float experience)
{
    addFurnaceRecipe(new ItemInstance(item, 1, -1), result, experience);
}

void FurnaceRecipes::addFurnaceRecipe(Block* block, ItemInstance* result, float experience)
{
    addFurnaceRecipe(Item::byBlock(block), result, experience);
}

void FurnaceRecipes::staticCtor()
{
    gFurnaceRecipes = new FurnaceRecipes();
}

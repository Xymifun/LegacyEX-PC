#include "game/item/ArmorDyeRecipe.hpp"

#include <algorithm>
#include <cstring>
#include <vector>

#include "game/core/DyeColor.hpp"
#include "game/entity/mob/HorseArmorItem.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"

ArmorDyeRecipe::~ArmorDyeRecipe()
{
}

bool ArmorDyeRecipe::matches(boost::shared_ptr<CraftingContainer> craftingContainer, Level*)
{
    not_null_ptr<ItemInstance> armor = ItemInstance::EMPTY;
    std::vector<not_null_ptr<ItemInstance> > dyes;

    for (unsigned int i = 0; i < craftingContainer->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (item->isEmpty())
        {
            continue;
        }

        ArmorItem* armorItem = dynamic_cast<ArmorItem*>(item->getItem());
        HorseArmorItem* horseArmorItem = dynamic_cast<HorseArmorItem*>(item->getItem());
        if (armorItem != NULL || horseArmorItem != NULL)
        {
            if (((armorItem != NULL && armorItem->getMaterial() == ArmorItem::ArmorMaterial::LEATHER)
                 || (horseArmorItem != NULL && horseArmorItem->isDyeable()))
                && armor->isEmpty())
            {
                armor = item;
            }
            else
            {
                return false;
            }
        }
        else if (item->getItem() == Items::dye)
        {
            dyes.push_back(item);
        }
        else
        {
            return false;
        }
    }

    return !armor->isEmpty() && !dyes.empty();
}

not_null_ptr<ItemInstance> ArmorDyeRecipe::assembleDyedArmor(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    unsigned int i;
    not_null_ptr<ItemInstance> armor = ItemInstance::EMPTY;
    ArmorItem* armorItem = NULL;
    HorseArmorItem* horseArmorItem = NULL;
    int red = 0;
    int green = 0;
    int blue = 0;
    int totalBrightness = 0;
    int colorCount = 0;

    if (craftingContainer)
    {
        for (i = 0; i < craftingContainer->getContainerSize(); ++i)
        {
            not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
            if (item->isEmpty())
            {
                continue;
            }

            if (dynamic_cast<ArmorItem*>(item->getItem()) != NULL
                || dynamic_cast<HorseArmorItem*>(item->getItem()) != NULL)
            {
                armorItem = dynamic_cast<ArmorItem*>(item->getItem());
                horseArmorItem = dynamic_cast<HorseArmorItem*>(item->getItem());
                if (!((armorItem != NULL && armorItem->getMaterial() == ArmorItem::ArmorMaterial::LEATHER)
                      || (horseArmorItem != NULL && horseArmorItem->isDyeable()))
                    || !armor->isEmpty())
                {
                    return ItemInstance::EMPTY;
                }

                armor = item->copy();
                armor->setCount(1);

                int color = -1;
                if (armorItem != NULL && armorItem->hasCustomColor(item))
                {
                    color = armorItem->getColor(armor, 0);
                }
                else if (horseArmorItem->hasCustomColor(item))
                {
                    color = horseArmorItem->getColor(armor, 0);
                }

                if (color != -1)
                {
                    float colorRed = static_cast<float>((color >> 16) & 255) / 255.0f;
                    float colorGreen = static_cast<float>((color >> 8) & 255) / 255.0f;
                    float colorBlue = static_cast<float>(color & 255) / 255.0f;
                    red += static_cast<int>(colorRed * 255.0f);
                    green += static_cast<int>(colorGreen * 255.0f);
                    blue += static_cast<int>(colorBlue * 255.0f);
                    const float& darkerChannel = std::max(colorGreen, colorBlue);
                    float brightestChannel = colorRed;
                    if (brightestChannel < darkerChannel)
                    {
                        brightestChannel = darkerChannel;
                    }
                    totalBrightness += static_cast<int>(brightestChannel * 255.0f);
                    ++colorCount;
                }
            }
            else if (item->getItem() == Items::dye)
            {
                arrayWithLength<const float> colors =
                    DyeColor::byItemData(item->getAuxValue())->getTextureDiffuseColors();
                int colorRed = static_cast<int>(colors.data[0] * 255.0f);
                int colorGreen = static_cast<int>(colors.data[1] * 255.0f);
                int colorBlue = static_cast<int>(colors.data[2] * 255.0f);
                const int& darkerChannel = std::max(colorGreen, colorBlue);
                int brightest = colorRed;
                if (brightest < darkerChannel)
                {
                    brightest = darkerChannel;
                }
                red += colorRed;
                green += colorGreen;
                blue += colorBlue;
                totalBrightness += brightest;
                ++colorCount;
            }
            else
            {
                return ItemInstance::EMPTY;
            }
        }
    }

    if (armorItem == NULL && horseArmorItem == NULL)
    {
        return ItemInstance::EMPTY;
    }

    red /= colorCount;
    green /= colorCount;
    blue /= colorCount;
    float averageBrightness = static_cast<float>(totalBrightness) / static_cast<float>(colorCount);
    const int& darkerChannel = std::max(green, blue);
    int brightestChannel = red;
    if (brightestChannel < darkerChannel)
    {
        brightestChannel = darkerChannel;
    }
    float brightest = static_cast<float>(brightestChannel);
    red = static_cast<int>(static_cast<float>(red) * averageBrightness / brightest);
    green = static_cast<int>(static_cast<float>(green) * averageBrightness / brightest);
    blue = static_cast<int>(static_cast<float>(blue) * averageBrightness / brightest);
    int color = red;
    color = (color << 8) + green;
    color = (color << 8) + blue;

    if (armorItem != NULL)
    {
        armorItem->setColor(armor, color);
    }
    else
    {
        horseArmorItem->setColor(armor, color);
    }

    return armor;
}

not_null_ptr<ItemInstance> ArmorDyeRecipe::assemble(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    return assembleDyedArmor(craftingContainer);
}

int ArmorDyeRecipe::size()
{
    return 10;
}

ItemInstance* ArmorDyeRecipe::getResultItem()
{
    return ItemInstance::EMPTY.get();
}

ItemInstance* ArmorDyeRecipe::getIngredientItem(int, int)
{
    return ItemInstance::EMPTY.get();
}

NonNullList<not_null_ptr<ItemInstance> > ArmorDyeRecipe::getRemainingItems(
    boost::shared_ptr<CraftingContainer> craftingContainer)
{
    NonNullList<not_null_ptr<ItemInstance> > result = NonNullList<not_null_ptr<ItemInstance> >::withSize(
        craftingContainer->getContainerSize(), ItemInstance::EMPTY);

    for (unsigned int i = 0; i < craftingContainer->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (item->getItem()->hasCraftingRemainingItem())
        {
            result.set(i, not_null_ptr<ItemInstance>(new ItemInstance(item->getItem()->getCraftingRemainingItem())));
        }
    }
    return result;
}

Recipe::_eGroupType ArmorDyeRecipe::getGroup()
{
    return Recipe::eGroupType_None;
}

bool ArmorDyeRecipe::recipeRequires(int)
{
    return false;
}

void ArmorDyeRecipe::recipeRequires(INGREDIENTS_REQUIRED* ingredientsRequired)
{
    int* source = new int[9];
    int* itemIdsScratch = new int[9];
    int* itemCountsScratch = new int[9];
    int* auxValuesScratch = new int[9];
    int* itemTagsScratch = new int[9];
    std::memset(itemIdsScratch, 0, 9 * sizeof(int));
    std::memset(itemCountsScratch, 0, 9 * sizeof(int));
    std::memset(auxValuesScratch, -1, 9 * sizeof(int));
    std::memset(itemTagsScratch, 0, 9 * sizeof(int));
    std::memset(source, 0, 9 * sizeof(int));

    ingredientsRequired->itemIds = new int[0];
    ingredientsRequired->itemCounts = new int[0];
    ingredientsRequired->auxValues = new int[0];
    ingredientsRequired->itemTags = new CompoundTag*[0];
    ingredientsRequired->craftingGrid = new int[9];
    ingredientsRequired->recipe = this;
    ingredientsRequired->available[0] = false;
    ingredientsRequired->available[2] = false;
    ingredientsRequired->available[1] = false;
    ingredientsRequired->requiresCraftingTable = 0;
    ingredientsRequired->available[3] = false;
    ingredientsRequired->numIngredients = 0;
    std::memcpy(ingredientsRequired->craftingGrid, source, 9 * sizeof(int));

    delete[] itemIdsScratch;
    delete[] itemCountsScratch;
    delete[] auxValuesScratch;
    delete[] itemTagsScratch;
    delete[] source;
}

boost::shared_ptr<CraftingContainer> ArmorDyeRecipe::generateExpectedCraftSlots()
{
    return boost::shared_ptr<CraftingContainer>();
}
